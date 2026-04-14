echo "ARP:"
if diff ./test_output/ArpTest.out ./output/Arp.out --ignore-all-space -B -u; then
    echo "  No differences"
fi

echo "HTTP"
if diff ./test_output/Http.out ./output/Http.out --ignore-all-space -B -u; then
    echo "  No differences"
fi

echo "IP_bad_checksum"
if diff ./test_output/IP_bad_checksum.out ./output/IP_bad_checksum.out --ignore-all-space -B -u; then
    echo "  No differences"
fi

echo "Large Mix"
if diff ./test_output/largeMix.out ./output/largeMix.out --ignore-all-space -B -u; then
    echo "  No differences"
fi

echo "Large Mix 2"
if diff ./test_output/largeMix2.out ./output/largeMix2.out --ignore-all-space -B -u; then
    echo "  No differences"
fi

echo "Mix With IP Options"
if diff ./test_output/mix_withIPoptions.out ./output/mix_withIPoptions.out --ignore-all-space -B; then
    echo "  No differences"
fi

echo "Ping"
if diff ./test_output/PingTest.out ./output/PingTest.out --ignore-all-space -B -u; then
    echo "  No differences"
fi

echo "smallTCP"
if diff ./test_output/smallTCP.out ./output/smallTCP.out --ignore-all-space -B -u; then
    echo "  No differences"
fi

echo "TCP_bad"
if diff ./test_output/TCP_bad_checksum.out ./output/TCP_bad_checksum.out --ignore-all-space -B -u; then
    echo "  No differences"
fi

echo "UDP"
if diff ./test_output/UDPfile.out ./output/UPDFile.out --ignore-all-space -B -u; then
    echo "  No differences"
fi