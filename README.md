# Painless TPM updates

[DesignFirst](https://designfirst.ee) offers the world's first TPM update service - EnactTrust.

Two flavors:

* RTOS & Baremetal - [Contact us](mailto:info@enacttrust.com)
* Linux - this repository

## Quick start

Try now on your Linux board. Download:

`wget https://github.com/EnactTrust/releases/download/v1.0.1/enact`

and run `sudo ./enact`. No account required, no compilation, it is that easy.

Note: sudo is required on some Linux systems for access to the TPM device.

## Supported Hardware

* Infineon
  * SLB 9672
  * SLB 9673
  * SLB 9670, SLI 9670, SLM 9670 (BETA, contact us)
* Nuvoton
  * NPCT7xx (Requires approval, contact us)

Contact us at our [email](mailto:info@enacttrust.com).

## How is EnactTrust different?

EnactTrust performs TPM 2.0 updates without manual steps. In comparision, other tools require multiple manual steps. Enact does everything in a single step.

[Download here](https://github.com/EnactTrust/releases) a static binary of `enact` that works on any modern ARM platform with Linux. The source code of this version is available in this GitHub repository.

Our embedded variant for RTOS environments is proprietary and requires a license. Contact us at our [email](mailto:info@enacttrust.com) for more information.

## Software variants

EnactTrust comes in two flavors:

* Embedded (requires a commercial license, no external dependencies)
* Non-embedded (this open-source version, uses the wolfTPM library)

For example:

* FreeRTOS-based system requires a commercial license. We provide an update component tailored to your system and without external dependencies.
* Linux-based system can use either the open-source variant (here) or purchase the embedded component that requires no external libraries.

## TPM updates without internet connectivity

We offer update bundles for systems without internet connectivity. Contact us at our [email](mailto:info@enacttrust.com) for update bundles tailored to your systems.

## Building it yourself

Follow these three steps:

1. Make sure your build system is set up

```
sudo apt-get update
```

```
sudo apt-get install -y build-essential coreutils bsdmainutils git
```

```
sudo apt-get install -y automake libtool libcurl4-openssl-dev libssl-dev
```

2. Install the wolfTPM library

Hashing the update manifest depends on having the wolfssl library

```
git clone --depth 1 https://github.com/wolfSSL/wolfssl.git
```

```
cd wolfssl
./autogen.sh
./configure --enable-wolftpm --disable-examples
make
sudo make install
sudo ldconfig
```

Now we can install the wolfTPM library

```
git clone --depth 1 https://github.com/wolfSSL/wolfTPM.git
```

```
cd wolfTPM
./autogen.sh
./configure --enable-devtpm --enable-infineon --disable-examples
make
sudo make install
sudo ldconfig
```

3. Build and run EnactTrust

```
git clone https://github.com/EnactTrust/enact.git
```

```
cd enact
make
./sudo enact
```

By default `EnactTrust` works without the need for a user account.

### Example output

```
~/enact $ sudo ./enact

=== EnactTrust TPM Updater === (version 1.0.1)

Starting EnactTrust...    Success
Downloading TPM update package... Success
Downloading TPM update license... Success
	 *** This TPM is manufactured by IFX ***
	 *** Please read the vendor license  ***
	 *** available as TPMlicense.txt     ***
TPM update start
TPM update: Extracting...
Found group 00000004
Manifest size is 3245
Data size is 925871
Wrote 3245 bytes to IFXmanifest.bin
Wrote 925871 bytes to IFXdata.bin
Infineon Firmware Update Tool
Mfg IFX (1), Vendor SLB9672, Fw 15.24 (0x4a0a)
Operational mode: Normal TPM operational mode (0x0)
KeyGroupId 0x4, FwCounter 1248 (249 same)
Firmware Update (normal mode):
Firmware Update Complete

=== Success - TPM is updated ===

Do you want an EnactTrust account? (Y/N) 
...

=== Account is registered ===

Your EnactTrust API token is sent to dimi@tpm.dev

=== Important - Reboot the system to finish the update ===

Goodbye
```

Let us know if you have any issues at our [email](mailto:info@enacttrust.com "contact us over email").

## Built With

The Linux version of EnactTrust uses:

* [wolfTPM](https://github.com/wolfssl/wolfTPM)
* [libcurl](https://github.com/curl/curl)

The `embedded` version of EnactTrust uses no external dependencies.

## Our Story

The original concept of EnactTrust emerged during 2017 and involves the largest trade fair for embedded systems - "Embedded World" in Nuremberg, Germany. For the very first time there was a dedicated Trusted Platform Module (TPM) track. Presenters included managers from ARM, OnSemi and other industry leaders. Surprisingly, no one from the five speakers talked about Trusted Computing or mentioned the use of TPM 2.0 modules.

Today, TPM 2.0 modules are used everywhere and we are seeing the rising adoption of TPM 2.0 in embedded systems: EV Chargers, Smart Metering, etc.

In early 2018, DesignFirst was founded. Our company solve the struggles with using hardware security and Trusted Platform Modules. We proudly host TPM.dev, the largest forum dedicated to hardware security. DesignFirst is an active member of the Trusted Computing Group (TCG) and we contribute to the Internet of Things (IoT) Workgroup.

## Contact us

Send us an [email](mailto:info@enacttrust.com "contact us over email") with your questions and we will respond. Alternatively, you could also use [TPM.dev](https://www.tpm.dev "TPM.dev community forum") forum.

DesignFirst OU, Copyright 2025