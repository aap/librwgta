#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>

#include <rw.h>
#include <args.h>

#include <rwgta.h>
#include <collision.h>

using namespace rw;

char *argv0;

#define COLL 0x4C4C4F43

void
readColFile(Stream *stream)
{
	struct {
		uint32 ident;
		uint32 size;
	} header;
	while(1){
		if(stream->read(&header, 8) == 0 ||
		   header.ident != COLL)
			return;
		uint8 *buf = new uint8[header.size];
		stream->read(buf, header.size);
		CColModel *colmodel = new CColModel;
		printf("%s\n", buf);
		// SA only: *(uint16)(buf+22) == object id
		readColModel(colmodel, buf+24);
		delete colmodel;
		delete[] buf;
	}
}

void
usage(void)
{
	fprintf(stderr, "usage: %s in.col\n", argv0);
	exit(1);
}

int
main(int argc, char *argv[])
{
	gta::attachPlugins();
	ARGBEGIN{
	}ARGEND;
	if(argc < 1)
		usage();

	StreamFile in;
	if(in.open(argv[0], "rb") == NULL)
		return 1;

	readColFile(&in);

	in.close();

	return 0;
}