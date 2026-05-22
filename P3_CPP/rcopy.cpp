#include "udpClient.h"

void checkArgs(int argc, char * argv[], UDPClientArgs& args);

int main (int argc, char *argv[])
{
	__PRINTF_DBG("DEBUG_MODE\n");
	UDPClientArgs args{};
	
	checkArgs(argc, argv, args);

	UDPClient client(args);

	client.run();
	
	return 0;
}

/*
	rcopy from-filename to-filename window-size buffer-size error-rate remote-machine remote-port

Arguments:
		from-filename: local file to upload.
		to-filename: file created by the server.
		window-size: number of packets in the sliding window.
		buffer-size: number of file-data bytes per data packet.
		error-rate: percent of packets in error, as a float.
		remote-machine: machine running the server.
		remote-port: server port.
*/

void checkArgs(int argc, char * argv[], UDPClientArgs& args)
{
    /* check command line arguments  */
	if (argc != 8)
	{
		printf("usage: %s from-filename to-filename window-size buffer-size error-rate remote-machine remote-port\n", argv[0]);
		exit(1);
	}

	if (strlen(argv[1]) > MAX_FILENAME_LEN) {
		printf("from-file name too long\n");
		exit(1);
	}
	if (strlen(argv[2]) > MAX_FILENAME_LEN) {
		printf("to-file name too long\n");
		exit(1);
	}
	if ((uint16_t)atol(argv[4]) > MAX_PAYLOAD_SIZE) {
		printf("buffer-size too large\n");
		exit(1);
	}
	
	args.fromFilename = argv[1];
	args.toFilename = argv[2];
	args.windowSize = atol(argv[3]);
	args.bufferSize = atol(argv[4]);
	args.errorRate = atof(argv[5]);
	args.remoteMachine = argv[6];
	args.remotePort = atoi(argv[7]);
}

