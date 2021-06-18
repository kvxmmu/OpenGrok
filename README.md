# FreeGrok \[WIP\]

**OpenSource** dedicated ngrok analog.
**FreeGrok** is designed to be faster than any other project like it.

# Features

- ZSTD packets compression
- Client/server windows compatibility

# TODO

- Timeout handling
   
   For example, when a client sends an incomplete packet, FreeGrok will 
   wait for it forever, ofc if packet is special crafted to fill your RAM with trash,
   use FreeGrok only when you sure that this will not happen. This is not dangerous, but 
   I'll try to fix it, maybe tomorrow.
- Whitelists/blacklists
- Database based blacklists/whitelists
- p2p mode

# Dependencies

- Compiler that supports C++17
- CMake
- Epoll or IOCP

WARNING: precompiled windows dll library is only available on x64 arch, otherwise crosscompile libzstd by yourself

# question/answer

- How can I contact with the developer?

    [here](https://t.me/kvxmmu) my native language is **Russian**, but you can contact with 
    me using **English**
    
- Why should i use **FreeGrok** instead of for example **hamachi** or **Radmin**?

    There is no particular reason to use **FreeGrok** instead of any other program that can
    open your port through NAT, but if you have virtual or dedicated server
    with a __dedicated IPv4__ you can use the **FreeGrok** because it is faster or 
    you have lower ping to the server. Also **FreeGrok** has compression!


# Config format

**FreeGrok** uses ini format for configs

By default FreeGrok takes the config file from the /etc directory, but you can change it by editing 
constant **CONFIG_FILE_LOCATION** in src/freegrok/defaults.hpp

#### Example config

[here](freegrok.ini.example)


# Client

Client can be found [here](https://github.com/kvxmmu/freegrok_client)
