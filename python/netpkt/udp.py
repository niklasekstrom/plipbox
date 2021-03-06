from impacket.ImpactPacket import *

def handle_udp_echo(eth_pkt, my_ip, my_port):
  ip_pkt = eth_pkt.child()
  if not isinstance(ip_pkt, IP):
    # no IP packet
    return None
  udp_pkt = ip_pkt.child()
  if not isinstance(udp_pkt, UDP):
    # no ICMP packet
    return None
  # check target ip
  my_ip_str = ".".join(map(str,my_ip))
  tgt_ip = ip_pkt.get_ip_dst()
  if tgt_ip != my_ip_str:
    return None
  # check target port
  port = udp_pkt.get_uh_dport()
  if port != my_port:
    return None

  data = udp_pkt.child()

  # generate a result
  eth = Ethernet()
  ip = IP()
  udp = UDP()

  eth.set_ether_shost(eth_pkt.get_ether_dhost())
  eth.set_ether_dhost(eth_pkt.get_ether_shost())
  eth.set_ether_type(0x800)

  ip.set_ip_src(ip_pkt.get_ip_dst())
  ip.set_ip_dst(ip_pkt.get_ip_src())
  ip.set_ip_id(0x1234)

  udp.set_uh_sport(udp_pkt.get_uh_dport())
  udp.set_uh_dport(udp_pkt.get_uh_sport())

  udp.contains(data)

  eth.contains(ip)
  ip.contains(udp)

  return eth
