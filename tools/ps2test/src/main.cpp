#include <cstdio>
#include <cassert>

#include <rw.h>
#include <rwgta.h>

// getting undefined references otherwise :/
int *__errno() { return &errno; }

using namespace std;

void
dumpmem(uchar *src, int size)
{
	static uchar buf[100];
}

int
main()
{
	FlushCache(0);
	rw::version = 0x34000;
	rw::platform = rw::PLATFORM_PS2;

	rw::Engine::init();
	gta::attachPlugins();
	rw::Driver::open();
	rw::engine->loadTextures = 0;

	rw::uint32 len;
	rw::uint8 *data = rw::getFileContents("host:player.DFF", &len);
	rw::StreamMemory in;
	in.open(data, len);
	rw::findChunk(&in, rw::ID_CLUMP, NULL, NULL);
	rw::Clump *clump = rw::Clump::streamRead(&in);
	in.close();
	delete[] data;

	printf("hello %p\n", clump);
	for(;;)
		printf("");
	return 0;
}
