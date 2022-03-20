# smart-card-sdk
This is a C library that provides read and authentication to the acr122u smart reader.

## Arch Linux
Install `pcsclite` and then execute the following to move winscard.h
to the correct location.
```bash
sudo cp /usr/include/PCSC/* /usr/include/
```

I have realised this ^^ is not needed and is due to my incompotence, I will not fix this
though.

You also need to install `acsccid` for the ACR122U then remove pn533
after connecting it.
```bash
# Plug reader in
sudo rmmod pn533_usb && pn533

# Stop the pcscd if it is running
# sudo systemctl stop pcscd

sudo pcscd -f -T
````

## Building Mannual Test Code
```bash
# add a main function?
make
```

## Using The SDK
 1. Copy `sdk.h` and, `sdk.c`
 2. Add the linker arguments `pkg-config libpcsclite --libs`
 3. Enjoy.

