# ft_ping

## Compilation

```
make
```

## Usage

```
sudo ./ft_ping [options] <destination>
```
(ft_ping need sudo to bind socket)

```
Usage
  ping [options] <destination>

Options:
  <destination>      dns name or ip address
  -f                 flood ping
  -h                 print help and exit
  -I <interface>     either interface name or address
  -l <preload>       send <preload> number of packages while waiting replies
  -m <mark>          tag the packets going out
  -M <pmtud opt>     define mtu discovery, can be one of <do|dont|want>
  -n                 no dns name resolution
  -p <pattern>       contents of padding byte
  -Q <tclass>        use quality of service <tclass> bits
  -S <size>          use <size> as SO_SNDBUF socket option value
  -t <ttl>           define time to live
  -w <deadline>      reply wait <deadline> in seconds
  -W <timeout>       time to wait for response

IPv4 options:
  -T <timestamp>     define timestamp, can be one of <tsonly|tsandaddr|tsprespec>
```

### TODO-LIST
+ IPv6 ping
+ options:
```
-I <interface>
-l <preload>
-m <mark>
-M <pmtud opt>
-n
-p <pattern>
-Q <tclass>
-S <size>
-t <ttl>
-W <timeout>
-T <timestamp>
```

## Packet

Ping use icmp packet with a payload of 56 bytes by default, this project use this structure (defined in `ft_ping.h`):
```
typedef struct		s_icmp_packet
{
	struct iphdr	iphdr;
	struct icmphdr	icmphdr;
	char		payload[PAYLOAD_SIZE];
}			t_icmp_packet;
```
From wikipedia:
```
-----------------------------------------------------------------
|IPv4 datagram | Bits 0–7 | Bits 8–15 | Bits 16–23 | Bits 24–31 |
---------------|------------------------------------------------|
|              | Versions |    ToS    |         Length          |
|              |------------------------------------------------|
|              |    Identification    |     flags and offsets   |
|    Header    |------------------------------------------------|
|  (20 bytes)  |    TTL   |  Protocol |     Header checksum     |
|              |------------------------------------------------|
|              |              Source IP Address                 |
|              |------------------------------------------------|
|              |           Destination IP Address               |
|--------------|------------------------------------------------|
|  ICMP Header |   Type   |    Code   |        Checksum         |
|   (8 bytes)  |------------------------------------------------|
|              |                 Header data                    |
|--------------|------------------------------------------------|
| ICMP Payload |                 Payload data                   |
|  (56 bytes)  |                                                |
|----------------------------------------------------------------
```

## Documentation

+ <https://en.wikipedia.org/wiki/Ping_(networking_utility)>
+ <https://en.wikipedia.org/wiki/IPv4#Header>
+ <https://en.wikipedia.org/wiki/Internet_Control_Message_Protocol>
+ ip_icmp.h
+ icmphdr.h
