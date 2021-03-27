# OpenGrok \[WIP\]

**OpenSource** dedicated ngrok analog.
**OpenGrok** is designed to be faster than any other project like it.

# TODO

- Timeout handling
   
   For example, when a client sends an incomplete packet, OpenGrok will 
   wait for it forever, ofc if packet is special crafted to fill your RAM with trash,
   use OpenGrok only when you sure that this will not happen.
   
   maybe tomorrow i'll fix it
- Whitelists/blacklists
- Database based blacklists/whitelists
- Implement functionality

# Dependencies

- Compiler that supports C++11
- CMake
- epoll
- Berkley sockets
- libconfig++

# question/answer

- How i can contact with the developer?

    [here](https://t.me/kvxmmu), my native language is **Russian**, but you can contact with 
    me using **English**
    
- Why should i use **OpenGrok** instead of for example **hamachi** or **Radmin**?

    There is no particular reason to use **OpenGrok** instead of any other program that can
    open your port through NAT, but if you have virtual or dedicated server
    with a __dedicated IPv4__ you can use the **OpenGrok** because it is faster or 
    you have lower ping to the server


# Config format

**OpenGrok** uses libconfig++ to parse configs
[here](https://hyperrealm.github.io/libconfig/libconfig_manual.html#Configuration-Files) you can find the format documentation

By default OpenGrok takes the config file from the /etc directory, but you can change it by editing 
constant **CONFIG_FILE_LOCATION** in src/definitions.hpp

#### Fields
**listen_port** - port that server will listen(**16bit unsigned integer**), **6567** - by default

**server_name** - server signature(client can ask about server signature, you can write anything in this field)

**listen_addr** - bind address, **0.0.0.0** - by default

#### Example config

Example config is placed in config/opengrok.cfg

