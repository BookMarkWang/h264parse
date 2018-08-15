#include "h264_parse.hpp"

int main(int argc, char** argv)
{
	H264Parser parser(argv[1]);
	parser.start();
	std::cout << parser << std::endl;
}
