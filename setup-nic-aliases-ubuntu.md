# Set up NIC aliases for Ubuntu 18.04

## Setup `eno1` using DHCP and `eno1:1`, `eno1:2`, and `eno1:3` as static

```sh
sudo vi /etc/network/interfaces
```

```ini
auto lo
    iface lo inet loopback

auto eno1
    iface eno1 inet dhcp

auto eno1:1
    iface eno1:1 inet static
    name Ethernet alias LAN card
    address 10.161.29.11
    netmask 255.255.255.0
    broadcast 10.161.29.255
    network 10.161.29.0

auto eno1:2
    iface eno1:2 inet static
    name Ethernet alias LAN card
    address 10.161.30.11
    netmask 255.255.255.0
    broadcast 10.161.30.255
    network 10.161.30.0

auto eno1:3
    iface eno1:3 inet static
    name Ethernet alias LAN card
    address 10.161.31.11
    netmask 255.255.255.0
    broadcast 10.161.31.255
    network 10.161.31.0
```

## Restart the network

```sh
sudo /etc/init.d/networking restart
```

Example output

```sh
nvidia@develmachine2:~/projects/git_tmp$ sudo /etc/init.d/networking restart
[ ok ] Restarting networking (via systemctl): networking.service.
```
