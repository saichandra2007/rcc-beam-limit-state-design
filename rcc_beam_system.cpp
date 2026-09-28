#include <iostream>
#include <cmath>
#include <iomanip>
#include <string>

using namespace std;

// Modulus of elasticity for steel (N/mm^2)
const double ES = 200000.0; 

// Limiting neutral axis ratio (xu_max / d) per IS 456:2000 Table Cl. 38.1
double get_xu_max_ratio(double fy) {
    if (fy >= 550.0) return 0.44;
    if (fy >= 500.0) return 0.46;
    if (fy >= 415.0) return 0.48;
    return 0.53; // Fe 250
}

// Limiting moment factor (Mu_lim / (fck * b * d^2)) per IS 456
double get_mu_lim_factor(double fy) {
    if (fy >= 550.0) return 0.130;
    if (fy >= 500.0) return 0.133;
    if (fy >= 415.0) return 0.138;
    return 0.148; // Fe 250
}

// ============================================================================
//                               DESIGN MODE
// ============================================================================
void run_design_mode() {
    cout << "\n=======================================================\n";
    cout << "             MODE 1: RCC BEAM DESIGN                   \n";
    cout << "=======================================================\n";

    double b, D, clear_cover, bar_dia, fck, fy, Mu_kNm, Vu_kN;

    cout << "Enter beam width (b) in mm                    : ";
    if (!(cin >> b) || b <= 0) return;
    cout << "Enter overall depth (D) in mm                 : ";
    cin >> D;
    cout << "Enter nominal clear cover in mm (e.g., 25, 30): ";
    cin >> clear_cover;
    cout << "Enter assumed main rebar diameter in mm       : ";
    cin >> bar_dia;
    cout << "Enter Concrete Grade fck (e.g., 20, 25, 30)   : ";
    cin >> fck;
    cout << "Enter Steel Grade fy (e.g., 415, 500, 550)    : ";
    cin >> fy;
    cout << "Enter Factored Bending Moment Mu in kN-m      : ";
    cin >> Mu_kNm;
    cout << "Enter Factored Shear Force Vu in kN           : ";
    cin >> Vu_kN;

    double d = D - clear_cover - (bar_dia / 2.0);
    double Mu = Mu_kNm * 1e6; // Convert to N-mm
    double Vu = Vu_kN * 1e3;   // Convert to N

    double xu_max_ratio = get_xu_max_ratio(fy);
    double xu_max = xu_max_ratio * d;
    double k_lim = get_mu_lim_factor(fy);
    double Mu_lim = k_lim * fck * b * pow(d, 2);

    cout << "\n" << string(55, '-') << "\n";
    cout << fixed << setprecision(2);
    cout << "SECTION & MATERIAL SUMMARY:\n";
    cout << " - Width (b) = " << b << " mm | Overall Depth (D) = " << D << " mm\n";
    cout << " - Effective Depth (d) = " << d << " mm (Clear Cover = " << clear_cover << " mm)\n";
    cout << " - Concrete Grade = M" << fck << " | Rebar Grade = Fe" << fy << "\n";
    cout << " - Limiting Moment Capacity (Mu,lim) = " << (Mu_lim / 1e6) << " kN-m\n";
    cout << string(55, '-') << "\n";

    // --- Flexural Design ---
    if (Mu <= Mu_lim) {
        cout << "\n[RESULT] SINGLY REINFORCED SECTION IS SAFE (Mu <= Mu,lim)\n";
        
        // IS 456 formula for required tensile steel
        double term = 1.0 - (4.6 * Mu) / (fck * b * pow(d, 2));
        double Ast = (0.5 * fck / fy) * (1.0 - sqrt(term)) * b * d;

        // Minimum and Maximum reinforcement limits
        double Ast_min = (0.85 * b * d) / fy;
        double Ast_max = 0.04 * b * D;

        cout << " - Required Tensile Steel (Ast) = " << Ast << " sq.mm\n";
        if (Ast < Ast_min) {
            cout << "   (Ast is below Ast,min of " << Ast_min << " sq.mm; adopting Ast,min)\n";
            Ast = Ast_min;
        }
        cout << " - Code Minimum Limit (Ast,min) = " << Ast_min << " sq.mm\n";
        cout << " - Code Maximum Limit (Ast,max) = " << Ast_max << " sq.mm\n";

        // Detailing suggestion
        int num_bars = ceil(Ast / (M_PI * pow(bar_dia, 2) / 4.0));
        double Ast_provided = num_bars * (M_PI * pow(bar_dia, 2) / 4.0);
        cout << " - Detailing Suggestion         : Provide " << num_bars << " bars of " 
             << (int)bar_dia << " mm dia (Ast provided = " << Ast_provided << " sq.mm)\n";

        // --- Shear Design ---
        cout << "\n--- SHEAR DESIGN (IS 456:2000 Cl. 40) ---\n";
        double tau_v = Vu / (b * d);
        double pt = (Ast_provided / (b * d)) * 100.0;
        double tau_c = 0.25 * sqrt(fck) * (pt / (pt + 1.0)); 
        if (tau_c < 0.35) tau_c = 0.35;
        if (tau_c > 0.85) tau_c = 0.85;

        cout << " - Nominal Shear Stress (tau_v)  = " << tau_v << " N/mm^2\n";
        cout << " - Permissible Concrete (tau_c)  = " << tau_c << " N/mm^2\n";

        double stirrup_dia = 8.0;
        double Asv = 2.0 * (M_PI * pow(stirrup_dia, 2) / 4.0); // 2-legged 8mm
        double sv;

        if (tau_v <= tau_c) {
            cout << " - tau_v <= tau_c: Nominal minimum shear stirrups required.\n";
            sv = (0.87 * fy * Asv) / (0.4 * b);
        } else {
            cout << " - tau_v > tau_c: Shear reinforcement required for excess shear (Vus = Vu - tau_c*b*d).\n";
            double Vus = Vu - (tau_c * b * d);
            sv = (0.87 * fy * Asv * d) / Vus;
        }

        double sv_max = min(0.75 * d, 300.0);
        sv = min(sv, sv_max);
        double sv_round = floor(sv / 10.0) * 10.0;
        cout << " - Stirrup Recommendation        : Provide 2-legged 8 mm stirrups @ " 
             << sv_round << " mm c/c (Max limit = " << sv_max << " mm)\n";

    } else {
        cout << "\n[!] OVER-REINFORCED WARNING: Mu (" << Mu_kNm << " kN-m) > Mu,lim (" << (Mu_lim / 1e6) << " kN-m)\n";
        cout << "The section CANNOT be designed as a singly reinforced section per IS 456:2000.\n\n";
        cout << "Recommendations to resolve:\n";
        double d_req = sqrt(Mu / (k_lim * fck * b));
        cout << "1. Increase Depth: Required balanced depth d_req = " << d_req 
             << " mm (Overall Depth D >= " << ceil(d_req + clear_cover + bar_dia / 2.0) << " mm)\n";
        
        double Mu2 = Mu - Mu_lim;
        double d_dash = clear_cover + 10.0; // Centroid of compression steel
        double fsc = 0.87 * fy * 0.90;      // Typical design compressive stress
        double Asc = Mu2 / (fsc * (d - d_dash));
        double Ast1 = (k_lim * fck * b * d * d) / (0.87 * fy * (d - 0.42 * xu_max));
        double Ast2 = Mu2 / (0.87 * fy * (d - d_dash));
        double total_Ast = Ast1 + Ast2;

        cout << "2. Design as Doubly Reinforced Section:\n";
        cout << "   - Singly section capacity (Mu,lim)                 = " << (Mu_lim / 1e6) << " kN-m\n";
        cout << "   - Excess moment resisted by compression steel (Mu2)= " << (Mu2 / 1e6) << " kN-m\n";
        cout << "   - Compression Steel Required (Asc)                 = " << Asc << " sq.mm\n";
        cout << "   - Total Tension Steel Required (Ast)               = " << total_Ast << " sq.mm\n";
    }
}

// ============================================================================
//                               ANALYSIS MODE
// ============================================================================
void run_analysis_mode() {
    cout << "\n=======================================================\n";
    cout << "             MODE 2: RCC BEAM ANALYSIS                 \n";
    cout << "=======================================================\n";

    double b, D, clear_cover, bar_dia, fck, fy, Ast;
    int num_bars;

    cout << "Enter beam width (b) in mm                    : ";
    if (!(cin >> b) || b <= 0) return;
    cout << "Enter overall depth (D) in mm                 : ";
    cin >> D;
    cout << "Enter clear cover in mm (e.g., 25, 30)        : ";
    cin >> clear_cover;
    cout << "Enter main tension rebar diameter in mm       : ";
    cin >> bar_dia;
    cout << "Enter number of tension bars                  : ";
    cin >> num_bars;
    cout << "Enter Concrete Grade fck in N/mm^2 (e.g., 25) : ";
    cin >> fck;
    cout << "Enter Steel Grade fy in N/mm^2 (e.g., 415, 500): ";
    cin >> fy;

    double d = D - clear_cover - (bar_dia / 2.0);
    Ast = num_bars * (M_PI * pow(bar_dia, 2) / 4.0);

    double xu_max_ratio = get_xu_max_ratio(fy);
    double xu_max = xu_max_ratio * d;

    // Nominal neutral axis assuming steel yields
    double xu = (0.87 * fy * Ast) / (0.36 * fck * b);

    cout << "\n" << string(55, '-') << "\n";
    cout << fixed << setprecision(2);
    cout << "SECTION PROPERTIES:\n";
    cout << " - Width (b) = " << b << " mm | Overall Depth (D) = " << D << " mm\n";
    cout << " - Effective Depth (d) = " << d << " mm\n";
    cout << " - Tension Steel Area (Ast) = " << Ast << " sq.mm (" << num_bars << " bars of " << (int)bar_dia << " mm dia)\n";
    cout << " - Limiting Neutral Axis (xu,max) = " << xu_max << " mm (xu,max/d = " << xu_max_ratio << ")\n";
    cout << " - Nominal Calculated xu          = " << xu << " mm\n";
    cout << string(55, '-') << "\n";

    if (xu < xu_max) {
        // --- Under-Reinforced Section ---
        cout << "\n[CLASSIFICATION]: UNDER-REINFORCED SECTION (xu < xu,max)\n";
        cout << "Behavior: Steel yields first with large deflections before concrete failure (Ductile Failure).\n";
        double Mu = 0.87 * fy * Ast * (d - 0.42 * xu);
        cout << " - Actual Depth of Neutral Axis (xu) = " << xu << " mm\n";
        cout << " - Ultimate Moment of Resistance (Mu)= " << (Mu / 1e6) << " kN-m\n";

    } else if (abs(xu - xu_max) < 1e-3) {
        // --- Balanced Section ---
        cout << "\n[CLASSIFICATION]: BALANCED SECTION (xu = xu,max)\n";
        cout << "Behavior: Steel reaches yield strain simultaneously as concrete reaches 0.0035 strain.\n";
        double Mu_lim = 0.36 * fck * b * xu_max * (d - 0.42 * xu_max);
        cout << " - Depth of Neutral Axis (xu)        = " << xu_max << " mm\n";
        cout << " - Limiting Moment of Resistance (Mu)= " << (Mu_lim / 1e6) << " kN-m\n";

    } else {
        // --- Over-Reinforced Section ---
        cout << "\n[CLASSIFICATION]: OVER-REINFORCED SECTION (xu > xu,max)!\n";
        cout << "Behavior: Concrete crushes in compression before steel yields (Brittle Sudden Failure).\n";
        cout << "\nPlease select how you want to evaluate this section:\n";
        cout << " Codal Analysis (IS 456 Safety Cap: Restrict Mu to Mu,lim and suggest Doubly Reinforced)\n";
        cout << " Exact Theoretical Strain-Compatibility Analysis (Compute true xu, steel stress fst, and true Mu)\n";
        cout << "Enter your choice (1 or 2): ";
        
        int choice;
        cin >> choice;

        if (choice == 1) {
            cout << "\n--- OPTION 1: CODAL RESTRICTION & DOUBLY REINFORCED RECOMMENDATION ---\n";
            double Mu_lim = 0.36 * fck * b * xu_max * (d - 0.42 * xu_max);
            cout << "Per IS 456:2000, design moment of over-reinforced sections must be capped at Mu,lim\n";
            cout << "to prevent sudden catastrophic concrete crushing without warning:\n";
            cout << " - Permissible Design Moment (Mu,lim) = " << (Mu_lim / 1e6) << " kN-m\n";
            cout << " - Effective xu considered by code   = " << xu_max << " mm\n";
            
            double balanced_Ast = (0.36 * fck * b * xu_max) / (0.87 * fy);
            double excess_Ast = Ast - balanced_Ast;
            cout << "\n[RECOMMENDATION FOR DOUBLY REINFORCED]:\n";
            cout << " - Balanced Tension Steel capacity for this size = " << balanced_Ast << " sq.mm\n";
            cout << " - Unutilized Excess Tension Steel in section    = " << excess_Ast << " sq.mm\n";
            cout << " - To utilize the full section safely, provide compression reinforcement (Asc)\n";
            cout << "   of approximately " << excess_Ast << " sq.mm near top face with lateral ties.\n";
            cout << "   This shifts the neutral axis back down to xu,max and guarantees ductile failure.\n";

        } else if (choice == 2) {
            cout << "\n--- OPTION 2: EXACT STRAIN COMPATIBILITY (TRUE xu & TRUE Mu) ---\n";
            cout << "Since steel does not reach yield strain (0.002 + 0.87*fy/Es), steel stress fst < 0.87*fy.\n";
            cout << "Equating Internal Compression C = Tension T:\n";
            cout << "   0.36 * fck * b * xu = Ast * fst\n";
            cout << "From linear strain compatibility: epsilon_st = 0.0035 * (d - xu) / xu\n";
            cout << "Setting fst = Es * epsilon_st gives the quadratic equation:\n";
            cout << "   0.36 * fck * b * xu^2 + (0.0035 * Es * Ast) * xu - (0.0035 * Es * Ast * d) = 0\n";

            double A_quad = 0.36 * fck * b;
            double B_quad = 0.0035 * ES * Ast;
            double C_quad = -0.0035 * ES * Ast * d;

            double disc = B_quad * B_quad - 4.0 * A_quad * C_quad;
            double true_xu = (-B_quad + sqrt(disc)) / (2.0 * A_quad);

            double eps_st = 0.0035 * (d - true_xu) / true_xu;
            double fst = ES * eps_st;
            if (fst > 0.87 * fy) fst = 0.87 * fy;

            double true_Mu = 0.36 * fck * b * true_xu * (d - 0.42 * true_xu);
            double Mu_lim = 0.36 * fck * b * xu_max * (d - 0.42 * xu_max);

            cout << "\n[EXACT NUMERICAL RESULTS]:\n";
            cout << " - True Depth of Neutral Axis (xu)       = " << true_xu << " mm\n";
            cout << " - True Strain in Steel (epsilon_st)     = " << setprecision(5) << eps_st << "\n" << setprecision(2);
            cout << " - Yield Strain Threshold (0.002+0.87fy/Es)= " << setprecision(5) << (0.002 + 0.87 * fy / ES) << "\n" << setprecision(2);
            cout << " - Actual Steel Stress (fst)             = " << fst << " N/mm^2 (vs 0.87*fy = " << 0.87 * fy << " N/mm^2)\n";
            cout << " - True Section Moment Capacity (Mu)     = " << (true_Mu / 1e6) << " kN-m\n";
            cout << "\n Note: While theoretical Mu is " << (true_Mu / 1e6) << " kN-m, IS 456 strictly mandates\n";
            cout << " restricting design capacity to Mu,lim (" << (Mu_lim / 1e6) << " kN-m) to prevent brittle failure.\n";
        }
    }
}

// ============================================================================
//                               MAIN MENU
// ============================================================================
int main() {
    int mode = 0;
    while (true) {
        cout << "\n=======================================================\n";
        cout << "      RCC BEAM LIMIT STATE SYSTEM (IS 456:2000)        \n";
        cout << "=======================================================\n";
        cout << " 1. DESIGN a Singly Reinforced RCC Beam (Given Loads)  \n";
        cout << " 2. ANALYSE an Existing RCC Beam Section (Given Steel) \n";
        cout << " 3. Exit                                               \n";
        cout << "Enter your choice (1, 2, or 3): ";
        if (!(cin >> mode)) break;

        if (mode == 1) {
            run_design_mode();
        } else if (mode == 2) {
            run_analysis_mode();
        } else if (mode == 3) {
            cout << "\nExiting RCC Beam System. Happy Designing!\n";
            break;
        } else {
            cout << "Invalid selection! Please enter 1, 2, or 3.\n";
        }
    }
    return 0;
}
