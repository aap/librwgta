all: colcode_vs_gl3.inc colcode_fs_gl3.inc ps2Building_vs_gl3.inc ps2Building_fs_gl3.inc

colcode_vs_gl3.inc: colcode.vert
	(echo 'const char *colcode_vert_src =';\
	 sed 's/..*/"&\\n"/' colcode.vert;\
	 echo ';') >colcode_vs_gl3.inc

colcode_fs_gl3.inc: colcode.frag
	(echo 'const char *colcode_frag_src =';\
	 sed 's/..*/"&\\n"/' colcode.frag;\
	 echo ';') >colcode_fs_gl3.inc

ps2Building_vs_gl3.inc: ps2Building.vert
	(echo 'const char *ps2Building_vert_src =';\
	 sed 's/..*/"&\\n"/' ps2Building.vert;\
	 echo ';') >ps2Building_vs_gl3.inc

ps2Building_fs_gl3.inc: ps2Building.frag
	(echo 'const char *ps2Building_frag_src =';\
	 sed 's/..*/"&\\n"/' ps2Building.frag;\
	 echo ';') >ps2Building_fs_gl3.inc
