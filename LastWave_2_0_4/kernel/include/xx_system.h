

extern char *XXGetHistoryFile(void);

extern char *XXConvertFilename(char *filename);

extern void XXStartup(int *argc, char ***argv);

extern void XXGetSystemInfo(char **home,char **system,char **computer,char **termType,char **scriptDir, char **user);

extern char *XXGetFilenames(char *directory);
extern void XXGetFilenameInfo(char *filename,int *type,int *size);
extern void XXChangeDirectory(char *directory);
extern char XXCreateDirectory(char *directory,char *name);

enum {
  DirectoryFile,
  RegularFile,
  UnknownTypeFile
};
