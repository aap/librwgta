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

struct ColHeader {
	uint32 ident;
	uint32 size;
};

void
writeSingleCol(CColModel *colmodel, char *name)
{
	char colname[24];
	ColHeader header;
	uint8 *buf;
	StreamFile stream;
	stream.open("out.col", "wb");
	header.ident = COLL;
	header.size = writeColModel(colmodel, &buf)+24;
	stream.write(&header, 8);
	memset(colname, 0, 24);
	strncpy(colname, name, 24);
	stream.write(colname, 24);
	stream.write(buf, header.size-24);
	delete[] buf;
	stream.close();
}

void
readColFile(Stream *stream)
{
	ColHeader header;
	char name[24];

	printf("reading bundle\n");
	while(1){
		if(stream->read(&header, 8) == 0 ||
		   header.ident != COLL)
			return;
		uint8 *buf = new uint8[header.size];
		stream->read(buf, header.size);
		memcpy(name, buf, 24);
		CColModel *colmodel = new CColModel;
		printf("%s %x\n", name, header.size);
		// SA only: *(uint16)(buf+22) == object id
		readColModel(colmodel, buf+24);
		delete[] buf;

		writeSingleCol(colmodel, name);
		delete colmodel;
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