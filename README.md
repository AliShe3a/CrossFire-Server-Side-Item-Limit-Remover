# 🔓 CrossFire Server-Side Item Limit Remover (`cf_gamesrv` & `cf_hostsrv`)

A complete, open-source C++ runtime memory patching and function hooking solution designed to remove hardcoded item table limits on **CrossFire Server Binaries (`cf_gamesrv.exe` & `cf_hostsrv`)**.

This repository completes the open-source ecosystem started in **[Vortex Anti-Cheat](https://github.com/AliShe3a/VortexAC)**, which introduced the **Client-Side Runtime Memory Patcher** for `item.cft` and `bf005.ltc`. Instead of relying on blind, closed-source binaries with fixed limits, server administrators can now inspect, customize, and compile the entire server-side solution themselves.

---

## 🧠 Why Is This Needed?

When expanding `item.cft` and `bf005.ltc` on the client side, the native server binaries (`cf_gamesrv.exe` and `cf_hostsrv` via `object.dll`) will still fail to register new items, truncate item arrays, or crash on startup due to hardcoded static buffer boundaries and validation checks.

This solution dynamically patches memory offsets and detours internal item initialization functions at runtime without permanently corrupting your original executable files on disk.

---

## 🏗️ Technical Architecture

The solution consists of two dedicated C++ DLL modules:

1. **`cf_gamesrv` Patcher DLL (MinHook + Runtime Patching):**
   * Hooks internal item loading and validation routines inside `cf_gamesrv.exe` using **MinHook**.
   * Dynamically expands the server's item allocation limits in memory.
   * **Dependency:** Requires the **MinHook DLL** (`MinHook.x86.dll` / `minhook.dll`) to be present in the exact same directory as `cf_gamesrv.exe` and the compiled patcher DLL.

2. **`cf_hostsrv` Patcher DLL (Runtime Memory Patching):**
   * Attaches directly to **`object.dll`** inside the `cf_hostsrv` directory.
   * Patches hardcoded room/host item validation limits at runtime so custom items work seamlessly inside match instances.

---

## 🚀 Step-by-Step Deployment Guide (Using `Stud_PE`)

To make the server load the compiled DLLs automatically upon startup, inject them into the **Import Address Table (IAT)** using **Stud_PE**:

### 1️⃣ Setting up `cf_gamesrv`
1. Compile the `server-gamesrv` patcher project in Visual Studio (Release / x86).
2. Copy your compiled `.dll` **AND** the **MinHook DLL** into your `cf_gamesrv` folder (right next to `cf_gamesrv.exe`).
3. Open **`Stud_PE`** and load **`cf_gamesrv.exe`**.
4. Navigate to the **`Functions`** tab ➡️ right-click inside the Imported Functions area ➡️ select **`Add New Import`**.
5. Select your compiled GameSrv Patcher `.dll`, choose its exported dummy/init function, and click **`Add to list`**.
6. Click **`Save`** to update the PE header. *(Stud_PE automatically creates a `.bak` backup of your original executable).*

### 2️⃣ Setting up `cf_hostsrv`
1. Compile the `server-hostsrv` patcher project in Visual Studio (Release / x86).
2. Copy your compiled `.dll` into the `cf_hostsrv` folder (where **`object.dll`** is located).
3. Open **`Stud_PE`** and load **`object.dll`**.
4. Navigate to the **`Functions`** tab ➡️ right-click ➡️ select **`Add New Import`**.
5. Select your compiled HostSrv Patcher `.dll`, pick the exported function, and click **`Add to list`**.
6. Click **`Save`**.

---

## 🧩 Complete Open-Source Ecosystem (Client + Server)

| Component | Target Files / Modules | Method | Status / Link |
| :--- | :--- | :--- | :--- |
| **Client-Side Limit Remover** | `item.cft` & `bf005.ltc` | Runtime Memory Patching | Included in **[Vortex Anti-Cheat](https://github.com/AliShe3a/VortexAC)** |
| **Game Server Limit Remover** | `cf_gamesrv.exe` | MinHook Detour + Memory Patch | **Included in this repo** |
| **Host Server Limit Remover** | `cf_hostsrv` (`object.dll`) | Runtime Memory Patching | **Included in this repo** |

---

## 🛠️ Requirements & Tech Stack

* **Language:** C++ (Win32 API, x86 Architecture)
* **Hooking Library:** [MinHook](https://github.com/TsudaKageyu/minhook) *(Required in `cf_gamesrv` folder at runtime)*
* **PE IAT Editor:** `Stud_PE` (for automatic DLL loading)

---

## ⚠️ License & Terms of Use

Developed and open-sourced by **Ali She3a (Vortex)** for educational and private server development purposes.

1. **100% Free & Open-Source:** Feel free to study, modify the item cap values, and use this on your servers.
2. **Strictly NO Commercial Sale:** Selling this source code or selling compiled binaries derived from it is strictly prohibited.
3. **Contact:** Discord `ashe3a`
