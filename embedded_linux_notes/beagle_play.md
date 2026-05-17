## Complete Boot Flow with TIFS Details
### Stage 0: Power-On Reset
```bash
[Power applied to board]
         ↓
All processors held in reset:
- A53 cores: RESET
- R5 core: RESET  
- M4F core: RESET
- Security Enclave: RESET
         ↓
ROM code starts executing
```

### Stage 1: ROM Boot Loader (First 2 microseconds)
What happens:

1. ROM code starts on internal execution unit
1. ROM code does initial security setup:
    - Checks if secure boot is enabled (reads eFUSEs)
    - Initializes basic security features
1. ROM code loads tiboot3.bin from boot media into SRAM

Important: At this point, TIFS is NOT running yet. The Security Enclave is still in reset.

### Stage 2: R5 SPL Starts (tiboot3.bin) - TIFS Startup Happens Here!

**Timeline in R5 SPL: T=0ms: R5 SPL execution begins**
```bash
R5 SPL entry point:
1. Sets up R5 stack pointer
2. Initializes R5 caches
3. Clears BSS section
4. Sets up minimal exception handlers
```

**T=1ms: R5 SPL prepares to load TIFS**
```c
// Pseudo-code of what R5 SPL does:

// Step 1: Parse tiboot3.bin image format
parse_tiboot3_image() {
    // tiboot3.bin is a multi-binary container
    // It contains:
    // - TIFS firmware binary
    // - DM firmware binary  
    // - R5 SPL code (already running)
    // - Certificates
    
    find_tifs_blob();  // Locate TIFS binary within tiboot3.bin
    find_dm_blob();    // Locate DM binary
}
```

**T=2ms: R5 SPL loads TIFS into Security Enclave memory**
```c
// Step 2: Load TIFS firmware
load_tifs() {
    // Read TIFS binary from tiboot3.bin
    tifs_binary = extract_tifs_from_tiboot3();
    
    // The Security Enclave has its own isolated RAM
    // R5 has SPECIAL PERMISSION to write to it ONLY during boot
    
    // Write TIFS firmware to Security Enclave's RAM
    copy_to_secure_ram(tifs_binary, SECURITY_ENCLAVE_RAM_BASE);
    
    // This is a one-time operation!
    // After TIFS starts, this RAM becomes inaccessible
}
```
**T=3ms: R5 SPL releases Security Enclave from reset**
```c
// Step 3: Start TIFS
start_tifs() {
    // Send command to power management controller
    write_register(SEC_ENCLAVE_RESET_CONTROL, RELEASE_RESET);
    
    // Security Enclave processor wakes up!
    // It starts executing TIFS firmware from its own RAM
}
```

**T=4ms: TIFS firmware starts executing on Security Enclave**
```bash
Security Enclave now running:
┌─────────────────────────────────┐
│  TIFS Firmware Initialization   │
│  Running on Security Enclave    │
├─────────────────────────────────┤
│ 1. Initialize crypto hardware   │
│ 2. Load encryption keys from    │
│    OTP/eFUSE                     │
│ 3. Set up secure mailbox for    │
│    communication                 │
│ 4. Initialize key derivation     │
│ 5. Start TISCI message server   │
│ 6. Signal "READY" to R5         │
└─────────────────────────────────┘
```

**T=5ms: R5 SPL waits for TIFS to be ready**
```c
// Step 4: Wait for TIFS ready signal
wait_for_tifs() {
    // R5 polls a shared mailbox register
    while (read_register(TIFS_STATUS_REG) != TIFS_READY) {
        delay(100); // 100 microseconds
    }
    
    // TIFS is now ready to accept requests!
}
```

**T=6ms: R5 SPL establishes communication with TIFS**
```c
// Step 5: Open secure channel with TIFS
open_tifs_channel() {
    // R5 and TIFS communicate via message mailboxes
    // These are special hardware registers
    
    sec_proxy_init();  // Initialize secure proxy (mailbox hardware)
    
    // Send first message to TIFS: "Hello, I'm R5 SPL"
    send_tifs_message(MSG_VERSION_CHECK);
    
    // TIFS responds with its version
    tifs_version = receive_tifs_response();
}
```
From this point on, TIFS is ALWAYS RUNNING in the background!

### How TIFS Communication Works
TIFS runs independently on the Security Enclave. Other processors talk to it via message passing.

**The Message Mailbox System:**
```bash
┌─────────────────────────────────────────────────────┐
│                  AM62x SoC                          │
│                                                     │
│  ┌────────────┐                 ┌──────────────┐   │
│  │  R5 Core   │                 │  Security    │   │
│  │            │                 │  Enclave     │   │
│  │  Running:  │    MAILBOX      │  (HSM)       │   │
│  │  R5 SPL or │◄───────────────►│              │   │
│  │  DM        │   (Hardware)    │  Running:    │   │
│  │            │                 │  TIFS        │   │
│  └────────────┘                 └──────────────┘   │
│                                                     │
│  ┌────────────┐                                    │
│  │  A53 Core  │                                    │
│  │            │                                    │
│  │  Running:  │    Via DM (Proxy)                 │
│  │  SPL/      │───────────────┐                   │
│  │  U-Boot/   │               │                   │
│  │  Linux     │               ↓                   │
│  └────────────┘        ┌─────────────┐            │
│                        │     DM      │            │
│                        │  (R5 runs)  │            │
│                        │             │            │
│                        │  Forwards   │            │
│                        │  messages   │            │
│                        │  to TIFS    │            │
│                        └─────────────┘            │
│                                                    │
└────────────────────────────────────────────────────┘
```

### Communication Protocol (TISCI - TI System Control Interface):
**Example: R5 SPL wants TIFS to verify tispl.bin signature**
```c
// Step 1: R5 prepares message
struct tisci_msg_verify_image {
    uint32_t header;           // Message type: VERIFY_IMAGE
    uint64_t image_address;    // Where tispl.bin is in DDR
    uint32_t image_size;       // Size of tispl.bin
    uint32_t cert_address;     // Where certificate is
};

// Step 2: R5 sends message to TIFS via mailbox
send_to_mailbox(TIFS_MAILBOX, &msg);

// Step 3: TIFS receives message in its mailbox interrupt handler
// (This happens on Security Enclave)
tifs_mailbox_irq_handler() {
    msg = read_from_mailbox();
    
    switch(msg.header) {
        case VERIFY_IMAGE:
            // TIFS does the work:
            // 1. Read certificate from DDR
            cert = read_memory(msg.cert_address);
            
            // 2. Extract public key from certificate
            public_key = parse_x509_cert(cert);
            
            // 3. Read image hash from certificate
            expected_hash = cert.image_hash;
            
            // 4. Calculate actual hash of tispl.bin
            // Uses hardware crypto accelerator
            actual_hash = sha512_hw(msg.image_address, msg.image_size);
            
            // 5. Compare hashes
            if (actual_hash == expected_hash) {
                // 6. Verify signature using RSA/ECC
                if (verify_signature(cert, public_key)) {
                    result = AUTHENTICATION_SUCCESS;
                } else {
                    result = SIGNATURE_INVALID;
                }
            } else {
                result = HASH_MISMATCH;
            }
            
            // 7. Send response back to R5
            send_response_to_mailbox(R5_MAILBOX, result);
            break;
    }
}

// Step 4: R5 receives response
response = receive_from_mailbox(R5_MAILBOX);

if (response == AUTHENTICATION_SUCCESS) {
    // Safe to execute tispl.bin
    jump_to_tispl();
} else {
    // Security violation! Halt boot!
    panic("Image authentication failed!");
}
```

## TIFS Throughout the Boot Flow - Complete Timeline
### Phase 1: R8 SPL Stage
```bash
R5 SPL Timeline with TIFS:

T=0ms    R5 SPL starts
T=1ms    R5 loads TIFS to Security Enclave RAM
T=2ms    R5 releases Security Enclave from reset
T=3ms    TIFS starts running on Security Enclave
T=4ms    TIFS initialization complete
T=5ms    R5 opens TISCI channel with TIFS

         ┌──── TIFS IS NOW RUNNING ────┐
         │                             │
T=10ms   R5: "TIFS, give me boot cert" │
         TIFS: "Here's the cert"       │
         │                             │
T=15ms   R5: "TIFS, enable firewall    │
              for region XYZ"           │
         TIFS: "Done, firewall set"    │
         │                             │
T=20ms   R5: "TIFS, I'm about to       │
              initialize DDR"           │
         TIFS: "OK, monitoring"        │
         │                             │
T=100ms  R5: DDR init done              │
         R5: "TIFS, verify tispl.bin"  │
         TIFS: *reads tispl.bin*       │
         TIFS: *computes SHA-512*      │
         TIFS: *verifies RSA sig*      │
         TIFS: "VERIFIED ✓"            │
         │                             │
T=120ms  R5: Loads tispl.bin to DDR    │
         R5: Starts A53                │
         │                             │
         └─────────────────────────────┘
```
### Phase 2: A53 SPL Stage
```base
A53 SPL Timeline with TIFS:

T=0ms    A53 SPL starts
         
         ┌──── TIFS STILL RUNNING ─────┐
         │                             │
T=5ms    A53: Needs to enable clock    │
         A53 → DM: "Enable clock X"    │
         DM → TIFS: "Permission check" │
         TIFS: "Allowed ✓"             │
         DM → A53: "Clock enabled"     │
         │                             │
T=50ms   A53: "TIFS, verify u-boot.img"│
         TIFS: *validates signature*   │
         TIFS: "VERIFIED ✓"            │
         │                             │
T=60ms   A53: Loads U-Boot             │
         │                             │
         └─────────────────────────────┘
```

### Phase 3: U-Boot Stage
```bash
U-Boot Timeline with TIFS:

T=0ms    U-Boot starts
         
         ┌──── TIFS STILL RUNNING ─────┐
         │                             │
T=10ms   U-Boot needs to read eFUSE    │
         U-Boot → DM → TIFS:           │
             "Read eFUSE MAC address"  │
         TIFS: *reads secure eFUSE*    │
         TIFS: "Here's the value"      │
         │                             │
T=100ms  U-Boot: "TIFS, verify kernel  │
              signature" (if secure)    │
         TIFS: *validates*             │
         TIFS: "VERIFIED ✓"            │
         │                             │
         └─────────────────────────────┘
```
### Phase 4: Linux Runtime
```bash
Linux Runtime with TIFS:

         ┌──── TIFS STILL RUNNING ─────┐
         │                             │
         Linux crypto driver needs      │
         hardware crypto acceleration:  │
         │                             │
         Linux → TIFS: "Encrypt this   │
             data with AES-256"         │
         TIFS: *uses hardware AES*     │
         TIFS: "Here's encrypted data" │
         │                             │
         Linux needs secure random:     │
         Linux → TIFS: "Give me random │
             bytes from TRNG"           │
         TIFS: *reads HW random gen*   │
         TIFS: "Here are random bytes" │
         │                             │
         └────── ALWAYS RUNNING ────────┘
```

### Memory Map - Where TIFS Lives

```bash
AM62x Memory Map:

0x00000000 ┌─────────────────────────┐
           │   Boot ROM (128KB)      │
           │   (ROM code runs here)  │
0x00020000 ├─────────────────────────┤
           │   SRAM (256KB)          │
           │   (tiboot3.bin loaded)  │
0x00060000 ├─────────────────────────┤
           │   ...                   │
           │                         │
           
0x43000000 ┌─────────────────────────┐
           │ SECURITY ENCLAVE RAM    │◄─── TIFS runs HERE
           │ (Isolated, 128KB)       │     Cannot be accessed
           │ - TIFS code             │     by A53/R5/M4!
           │ - TIFS stack            │
           │ - Crypto keys           │
           │ - Secure workspace      │
0x43020000 └─────────────────────────┘

0x80000000 ┌─────────────────────────┐
           │   DDR RAM (2GB)         │
           │   - tispl.bin           │
           │   - u-boot.img          │
           │   - Linux kernel        │
           │   - Applications        │
0xFFFFFFFF └─────────────────────────┘
```

How TIFS is used:
- R5 SPL → TIFS: Verify tispl.bin signature
- A53 SPL → TIFS: Verify u-boot.img signature
- U-Boot → TIFS: Verify kernel signature
- Linux → TIFS: Crypto operations, random numbers