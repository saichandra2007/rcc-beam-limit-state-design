# RCC Beam Limit State Design & Analysis Tool (IS 456:2000)

A lightweight, high-performance computational engineering tool written in **C++** for the automated **Limit State Analysis and Design** of singly and doubly reinforced concrete beams in strict compliance with the Indian Standard code **IS 456:2000**.

---

## Key Features

### 1. Beam Design Mode (Given Factored Loads)
- Computes limiting moment capacity ($M_{u,\text{lim}}$) based on steel grade ($Fe250$, $Fe415$, $Fe500$, $Fe550$).
- If $M_u \le M_{u,\text{lim}}$: Automates tension steel calculation ($A_{st}$), verifies codal boundaries ($A_{st,\min}$ and $A_{st,\max}$), and suggests bar sizes.
- If $M_u > M_{u,\text{lim}}$: Flags over-reinforced section and provides exact requirements for:
  - Required increase in effective depth ($d_{\text{req}}$).
  - Compression steel ($A_{sc}$) and tension steel ($A_{st}$) for a doubly reinforced section.
- **Shear Design (Clause 40):** Computes nominal shear stress ($\tau_v$), permissible concrete shear strength ($\tau_c$), and outputs 2-legged 8 mm stirrup spacing with code maximum limits.

### 2. Beam Analysis Mode (Given Existing Section & Steel)
- Evaluates actual neutral axis depth ($x_u$) and compares with $x_{u,\max}$.
- Classifies section as **Under-reinforced**, **Balanced**, or **Over-reinforced**.
- **Over-Reinforced Dual Evaluation:**
  - **Codal Option:** Caps design moment at $M_{u,\text{lim}}$ per IS 456 to prevent brittle concrete crushing and outputs required compression steel ($A_{sc}$) to restore ductility.
  - **Exact Theoretical Strain-Compatibility:** Formulates and solves the non-linear quadratic compatibility equation ($0.36 f_{ck} b x_u = A_{st} f_{st}$) to determine the true neutral axis ($x_u$), true steel strain ($\epsilon_{st}$), actual steel stress ($f_{st} < 0.87 f_y$), and true flexural capacity.

---

## Technical Specifications & Design Codes
- **Design Standard:** IS 456:2000 (Plain and Reinforced Concrete - Code of Practice)
- **Programming Language:** C++ (C++11 or higher)
- **Compiler:** Clang / GCC (Cross-platform compatible)

---

## How to Build and Run

### On macOS / Linux:
```bash
# Clone the repository
git clone [https://github.com/](https://github.com/)<your-username>/rcc-beam-limit-state-design.git
cd rcc-beam-limit-state-design

# Compile using g++ or clang++
g++ -std=c++11 rcc_beam_system.cpp -o rcc_beam_system

# Run the executable
./rcc_beam_system
