all: simple_vs_gl3.inc colcode_vs_gl3.inc colcode_fs_gl3.inc \
	ps2Building_vs_gl3.inc ps2Building_fs_gl3.inc \
	ps2BuildingFX_vs_gl3.inc ps2Env_fs_gl3.inc \
	neoWorldIII_fs_gl3.inc neoWorldVC_fs_gl3.inc

simple_vs_gl3.inc: simple.vert
	(echo 'const char *simple_vert_src =';\
	 sed 's/..*/"&\\n"/' simple.vert;\
	 echo ';') >simple_vs_gl3.inc

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

ps2BuildingFX_vs_gl3.inc: ps2BuildingFX.vert
	(echo 'const char *ps2BuildingFX_vert_src =';\
	 sed 's/..*/"&\\n"/' ps2BuildingFX.vert;\
	 echo ';') >ps2BuildingFX_vs_gl3.inc

ps2Env_fs_gl3.inc: ps2Env.frag
	(echo 'const char *ps2Env_frag_src =';\
	 sed 's/..*/"&\\n"/' ps2Env.frag;\
	 echo ';') >ps2Env_fs_gl3.inc


neoWorldIII_fs_gl3.inc: neoWorldIII.frag
	(echo 'const char *neoWorldIII_frag_src =';\
	 sed 's/..*/"&\\n"/' neoWorldIII.frag;\
	 echo ';') >neoWorldIII_fs_gl3.inc

neoWorldVC_fs_gl3.inc: neoWorldVC.frag
	(echo 'const char *neoWorldVC_frag_src =';\
	 sed 's/..*/"&\\n"/' neoWorldVC.frag;\
	 echo ';') >neoWorldVC_fs_gl3.inc
