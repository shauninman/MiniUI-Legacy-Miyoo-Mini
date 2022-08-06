#include <stdio.h>
#include <stdlib.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <msettings.h>
#include <sys/types.h>
#include <dirent.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>

#include "../common/common.h"

///////////////////////////////////////

#define dump(msg) puts((msg));fflush(stdout);

///////////////////////////////////////

typedef struct Array {
	int count;
	int capacity;
	void** items;
} Array;

static Array* Array_new(void) {
	Array* self = malloc(sizeof(Array));
	self->count = 0;
	self->capacity = 8;
	self->items = malloc(sizeof(void*) * self->capacity);
	return self;
}
static void Array_push(Array* self, void* item) {
	if (self->count>=self->capacity) {
		self->capacity *= 2;
		self->items = realloc(self->items, sizeof(void*) * self->capacity);
	}
	self->items[self->count++] = item;
}
static void Array_unshift(Array* self, void* item) {
	if (self->count==0) return Array_push(self, item);
	Array_push(self, NULL); // ensures we have enough capacity
	for (int i=self->count-2; i>=0; i--) {
		self->items[i+1] = self->items[i];
	}
	self->items[0] = item;
}
static void* Array_pop(Array* self) {
	if (self->count==0) return NULL;
	return self->items[--self->count];
}
static void Array_reverse(Array* self) {
	int end = self->count-1;
	int mid = self->count/2;
	for (int i=0; i<mid; i++) {
		void* item = self->items[i];
		self->items[i] = self->items[end-i];
		self->items[end-i] = item;
	}
}
static void Array_free(Array* self) {
	free(self->items); 
	free(self);
}

static int StringArray_indexOf(Array* self, char* str) {
	for (int i=0; i<self->count; i++) {
		if (exactMatch(self->items[i], str)) return i;
	}
	return -1;
}
static void StringArray_free(Array* self) {
	for (int i=0; i<self->count; i++) {
		free(self->items[i]);
	}
	Array_free(self);
}

///////////////////////////////////////

typedef struct Hash {
	Array* keys;
	Array* values;
} Hash; // not really a hash

static Hash* Hash_new(void) {
	Hash* self = malloc(sizeof(Hash));
	self->keys = Array_new();
	self->values = Array_new();
	return self;
}
static void Hash_free(Hash* self) {
	StringArray_free(self->keys);
	StringArray_free(self->values);
	free(self);
}
static void Hash_set(Hash* self, char* key, char* value) {
	Array_push(self->keys, strdup(key));
	Array_push(self->values, strdup(value));
}
static char* Hash_get(Hash* self, char* key) {
	int i = StringArray_indexOf(self->keys, key);
	if (i==-1) return NULL;
	return self->values->items[i];
}

///////////////////////////////////////

enum EntryType {
	kEntryDir,
	kEntryPak,
	kEntryRom,
};
typedef struct Entry {
	char* path;
	char* name;
	char* unique;
	int type;
	int alpha; // index in parent Directory's alphas Array, which points to the index of an Entry in its entries Array :sweat_smile:
	
	int has_alt;
	int use_alt;
} Entry;

static Entry* Entry_new(char* path, int type) {
	char display_name[256];
	getDisplayName(path, display_name);
	Entry* self = malloc(sizeof(Entry));
	self->path = strdup(path);
	self->name = strdup(display_name);
	self->unique = NULL;
	self->type = type;
	self->alpha = 0;
	self->has_alt = type!=kEntryRom?0:-1;
	self->use_alt = type!=kEntryRom?0:-1;
	return self;
}
static void Entry_free(Entry* self) {
	free(self->path);
	free(self->name);
	if (self->unique) free(self->unique);
	free(self);
}

static int EntryArray_indexOf(Array* self, char* path) {
	for (int i=0; i<self->count; i++) {
		Entry* entry = self->items[i];
		if (exactMatch(entry->path, path)) return i;
	}
	return -1;
}
static int EntryArray_sortEntry(const void* a, const void* b) {
	Entry* item1 = *(Entry**)a;
	Entry* item2 = *(Entry**)b;
	return strcasecmp(item1->name, item2->name);
}
static void EntryArray_sort(Array* self) {
	qsort(self->items, self->count, sizeof(void*), EntryArray_sortEntry);
}

static void EntryArray_free(Array* self) {
	for (int i=0; i<self->count; i++) {
		Entry_free(self->items[i]);
	}
	Array_free(self);
}

///////////////////////////////////////

#define kIntArrayMax 27
typedef struct IntArray {
	int count;
	int items[kIntArrayMax];
} IntArray;
static IntArray* IntArray_new(void) {
	IntArray* self = malloc(sizeof(IntArray));
	self->count = 0;
	memset(self->items, 0, sizeof(int) * kIntArrayMax);
	return self;
}
static void IntArray_push(IntArray* self, int i) {
	self->items[self->count++] = i;
}
static void IntArray_free(IntArray* self) {
	free(self);
}

///////////////////////////////////////

typedef struct Directory {
	char* path;
	char* name;
	Array* entries;
	IntArray* alphas;
	// rendering
	int selected;
	int start;
	int end;
} Directory;

static int getIndexChar(char* str) {
	char i = 0;
	char c = tolower(str[0]);
	if (c>='a' && c<='z') i = (c-'a')+1;
	return i;
}

static void getUniqueName(Entry* entry, char* out_name) {
	char* filename = strrchr(entry->path, '/')+1;
	char emu_tag[256];
	getEmuName(entry->path, emu_tag);
	
	char *tmp;
	strcpy(out_name, entry->name);
	tmp = out_name + strlen(out_name);
	strcpy(tmp, " (");
	tmp = out_name + strlen(out_name);
	strcpy(tmp, emu_tag);
	tmp = out_name + strlen(out_name);
	strcpy(tmp, ")");
}

static void Directory_index(Directory* self) {
	int skip_index = exactMatch(Paths.fauxRecentDir, self->path) || prefixMatch(Paths.collectionsDir, self->path); // not alphabetized
	
	Entry* prior = NULL;
	int alpha = -1;
	int index = 0;
	for (int i=0; i<self->entries->count; i++) {
		Entry* entry = self->entries->items[i];
		if (prior!=NULL && exactMatch(prior->name, entry->name)) {
			if (prior->unique) free(prior->unique);
			if (entry->unique) free(entry->unique);
			
			char* prior_filename = strrchr(prior->path, '/')+1;
			char* entry_filename = strrchr(entry->path, '/')+1;
			if (exactMatch(prior_filename, entry_filename)) {
				char prior_unique[256];
				char entry_unique[256];
				getUniqueName(prior, prior_unique);
				getUniqueName(entry, entry_unique);
				
				prior->unique = strdup(prior_unique);
				entry->unique = strdup(entry_unique);
			}
			else {
				prior->unique = strdup(prior_filename);
				entry->unique = strdup(entry_filename);
			}
		}

		if (!skip_index) {
			int a = getIndexChar(entry->name);
			if (a!=alpha) {
				index = self->alphas->count;
				IntArray_push(self->alphas, i);
				alpha = a;
			}
			entry->alpha = index;
		}
		
		prior = entry;
	}
}

static Array* getRoot(void);
static Array* getRecents(void);
static Array* getCollection(char* path);
static Array* getDiscs(char* path);
static Array* getEntries(char* path);

static Directory* Directory_new(char* path, int selected) {
	char display_name[256];
	getDisplayName(path, display_name);
	
	Directory* self = malloc(sizeof(Directory));
	self->path = strdup(path);
	self->name = strdup(display_name);
	if (exactMatch(path, Paths.rootDir)) {
		self->entries = getRoot();
	}
	else if (exactMatch(path, Paths.fauxRecentDir)) {
		self->entries = getRecents();
	}
	else if (!exactMatch(path, Paths.collectionsDir) && prefixMatch(Paths.collectionsDir, path)) {
		self->entries = getCollection(path);
	}
	else if (suffixMatch(".m3u", path)) {
		self->entries = getDiscs(path);
	}
	else {
		self->entries = getEntries(path);
	}
	self->alphas = IntArray_new();
	self->selected = selected;
	Directory_index(self);
	return self;
}
static void Directory_free(Directory* self) {
	free(self->path);
	free(self->name);
	EntryArray_free(self->entries);
	IntArray_free(self->alphas);
	free(self);
}

static void DirectoryArray_pop(Array* self) {
	Directory_free(Array_pop(self));
}
static void DirectoryArray_free(Array* self) {
	for (int i=0; i<self->count; i++) {
		Directory_free(self->items[i]);
	}
	Array_free(self);
}

///////////////////////////////////////

typedef struct Recent {
	char* path; // NOTE: this is without the Paths.rootDir prefix!
	int available;
} Recent;
static int hasEmu(char* emu_name);
static Recent* Recent_new(char* path) {
	Recent* self = malloc(sizeof(Recent));

	char sd_path[256]; // only need to get emu name
	sprintf(sd_path, "%s%s", Paths.rootDir, path);

	char emu_name[256];
	getEmuName(sd_path, emu_name);
	
	self->path = strdup(path);
	self->available = hasEmu(emu_name);
	return self;
}
static void Recent_free(Recent* self) {
	free(self->path);
	free(self);
}

static int RecentArray_indexOf(Array* self, char* str) {
	for (int i=0; i<self->count; i++) {
		Recent* item = self->items[i];
		if (exactMatch(item->path, str)) return i;
	}
	return -1;
}
static void RecentArray_free(Array* self) {
	for (int i=0; i<self->count; i++) {
		Recent_free(self->items[i]);
	}
	Array_free(self);
}

///////////////////////////////////////

static Directory* top;
static Array* stack; // DirectoryArray
static Array* recents; // RecentArray

static int is_simple = 0;
static int quit = 0;
static int can_resume = 0;
static int should_resume = 0; // set to 1 on kButtonResume but only if can_resume==1
static char slot_path[256];

static int restore_depth = -1;
static int restore_relative = -1;
static int restore_selected = 0;
static int restore_start = 0;
static int restore_end = 0;

///////////////////////////////////////

#define kMaxRecents 24 // a multiple of all menu rows
static void saveRecents(void) {
	FILE* file = fopen(Paths.recentPath, "w");
	if (file) {
		for (int i=0; i<recents->count; i++) {
			Recent* recent = recents->items[i];
			fputs(recent->path, file);
			putc('\n', file);
		}
		fclose(file);
	}
}
static void addRecent(char* path) {
	path += strlen(Paths.rootDir); // makes paths platform agnostic
	int id = RecentArray_indexOf(recents, path);
	if (id==-1) { // add
		while (recents->count>=kMaxRecents) {
			Recent_free(Array_pop(recents));
		}
		Array_unshift(recents, Recent_new(path));
	}
	else if (id>0) { // bump to top
		for (int i=id; i>0; i--) {
			void* tmp = recents->items[i-1];
			recents->items[i-1] = recents->items[i];
			recents->items[i] = tmp;
		}
	}
	saveRecents();
}

static int hasEmu(char* emu_name) {
	// if (exactMatch(emu_name, "PAK")) return 1;
	char pak_path[256];
	sprintf(pak_path, "%s/Emus/%s.pak/launch.sh", Paths.paksDir, emu_name);
	if (exists(pak_path)) return 1;

	sprintf(pak_path, "%s/Emus/%s.pak/launch.sh", Paths.rootDir, emu_name);
	return exists(pak_path);
}
static void getEmuPath(char* emu_name, char* pak_path) {
	sprintf(pak_path, "%s/Emus/%s.pak/launch.sh", Paths.rootDir, emu_name);
	if (exists(pak_path)) return;
	sprintf(pak_path, "%s/Emus/%s.pak/launch.sh", Paths.paksDir, emu_name);
}
static int hasAlt(char* emu_name) {
	char pak_path[256];
	sprintf(pak_path, "%s/Emus/%s.pak/has-alt", Paths.paksDir, emu_name);
	return exists(pak_path);
}
static int hasCue(char* dir_path, char* cue_path) { // NOTE: dir_path not rom_path
	char* tmp = strrchr(dir_path, '/') + 1; // folder name
	sprintf(cue_path, "%s/%s.cue", dir_path, tmp);
	return exists(cue_path);
}
static int hasM3u(char* rom_path, char* m3u_path) { // NOTE: rom_path not dir_path
	char* tmp;
	
	strcpy(m3u_path, rom_path);
	tmp = strrchr(m3u_path, '/') + 1;
	tmp[0] = '\0';
	
	// path to parent directory
	char base_path[256];
	strcpy(base_path, m3u_path);
	
	tmp = strrchr(m3u_path, '/');
	tmp[0] = '\0';
	
	// get parent directory name
	char dir_name[256];
	tmp = strrchr(m3u_path, '/');
	strcpy(dir_name, tmp);
	
	// dir_name is also our m3u file name
	tmp = m3u_path + strlen(m3u_path); 
	strcpy(tmp, dir_name);

	// add extension
	tmp = m3u_path + strlen(m3u_path);
	strcpy(tmp, ".m3u");
	
	return exists(m3u_path);
}

static int Entry_hasAlt(Entry* self) {
	// has_alt can be set by getEntries()
	// but won't be set by getRecents()
	// otherwise delayed until selected
	if (self->has_alt==-1) {
		// check
		char emu_name[256];
		getEmuName(self->path, emu_name);
		self->has_alt = hasAlt(emu_name);
	}
	return self->has_alt;
}
static int Entry_useAlt(Entry* self) {
	// has to be checked on an individual basis
	// but delayed until selected
	
	if (self->use_alt==-1) {
		// check
		char emu_name[256];
		getEmuName(self->path, emu_name);
		
		char rom_name[256];
		char* tmp = strrchr(self->path, '/');
		if (tmp) strcpy(rom_name, tmp+1);
		
		char use_alt[256];
		sprintf(use_alt, "%s/.mmenu/%s/%s.use-alt", Paths.userdataDir, emu_name, rom_name);
		
		self->use_alt = exists(use_alt);
	}
	return self->use_alt;
}
static int Entry_toggleAlt(Entry* self) {
	if (!Entry_hasAlt(self)) return 0;

	self->use_alt = !Entry_useAlt(self);
	
	char emu_name[256];
	getEmuName(self->path, emu_name);
	
	char rom_name[256];
	char* tmp = strrchr(self->path, '/');
	if (tmp) strcpy(rom_name, tmp+1);
	
	char use_alt_path[256];
	sprintf(use_alt_path, "%s/.mmenu/%s/%s.use-alt", Paths.userdataDir, emu_name, rom_name);
	
	if (self->use_alt==1) touch(use_alt_path);
	else unlink(use_alt_path);
		
	return 1;
}

static int hasRecents(void) {
	int has = 0;
	
	Array* parent_paths = Array_new();
	if (exists(kChangeDiscPath)) {
		char sd_path[256];
		getFile(kChangeDiscPath, sd_path, 256);
		if (exists(sd_path)) {
			char* disc_path = sd_path + strlen(Paths.rootDir); // makes path platform agnostic
			Recent* recent = Recent_new(disc_path);
			if (recent->available) has += 1;
			Array_push(recents, recent);
		
			char parent_path[256];
			strcpy(parent_path, disc_path);
			char* tmp = strrchr(parent_path, '/') + 1;
			tmp[0] = '\0';
			Array_push(parent_paths, strdup(parent_path));
		}
		unlink(kChangeDiscPath);
	}
	
	FILE* file = fopen(Paths.recentPath, "r"); // newest at top
	if (file) {
		char line[256];
		while (fgets(line,256,file)!=NULL) {
			normalizeNewline(line);
			trimTrailingNewlines(line);
			if (strlen(line)==0) continue; // skip empty lines
			
			char sd_path[256];
			sprintf(sd_path, "%s%s", Paths.rootDir, line);
			if (exists(sd_path)) {
				if (recents->count<kMaxRecents) {
					// this logic replaces an existing disc from a multi-disc game with the last used
					char m3u_path[256];
					if (hasM3u(sd_path, m3u_path)) { // TODO: this might tank launch speed
						char parent_path[256];
						strcpy(parent_path, line);
						char* tmp = strrchr(parent_path, '/') + 1;
						tmp[0] = '\0';
						
						int found = 0;
						for (int i=0; i<parent_paths->count; i++) {
							char* path = parent_paths->items[i];
							if (prefixMatch(path, parent_path)) {
								found = 1;
								break;
							}
						}
						if (found) continue;
						
						Array_push(parent_paths, strdup(parent_path));
					}
					Recent* recent = Recent_new(line);
					if (recent->available) has += 1;
					Array_push(recents, recent);
				}
			}
		}
		fclose(file);
	}
	
	saveRecents();
	
	StringArray_free(parent_paths);
	return has>0;
}
static int hasCollections(void) {
	int has = 0;
	if (!exists(Paths.collectionsDir)) return has;
	
	DIR *dh = opendir(Paths.collectionsDir);
	struct dirent *dp;
	while((dp = readdir(dh)) != NULL) {
		if (hide(dp->d_name)) continue;
		has = 1;
		break;
	}
	closedir(dh);
	return has;
}
static int hasRoms(char* dir_name) {
	int has = 0;
	char emu_name[256];
	char rom_path[256];

	getEmuName(dir_name, emu_name);
	
	// check for emu pak
	if (!hasEmu(emu_name)) return has;
	
	// check for at least one non-hidden file (we're going to assume it's a rom)
	sprintf(rom_path, "%s/%s/", Paths.romsDir, dir_name);
	DIR *dh = opendir(rom_path);
	if (dh!=NULL) {
		struct dirent *dp;
		while((dp = readdir(dh)) != NULL) {
			if (hide(dp->d_name)) continue;
			has = 1;
			break;
		}
		closedir(dh);
	}
	// if (!has) printf("No roms for %s!\n", dir_name);
	return has;
}
static Array* getRoot(void) {
	Array* root = Array_new();
	
	if (hasRecents()) Array_push(root, Entry_new(Paths.fauxRecentDir, kEntryDir));
	
	DIR *dh;
	
	Array* entries = Array_new();
	dh = opendir(Paths.romsDir);
	if (dh!=NULL) {
		struct dirent *dp;
		char* tmp;
		char full_path[256];
		sprintf(full_path, "%s/", Paths.romsDir);
		tmp = full_path + strlen(full_path);
		Array* emus = Array_new();
		while((dp = readdir(dh)) != NULL) {
			if (hide(dp->d_name)) continue;
			if (hasRoms(dp->d_name)) {
				strcpy(tmp, dp->d_name);
				Array_push(emus, Entry_new(full_path, kEntryDir));
			}
		}
		EntryArray_sort(emus);
		Entry* prev_entry = NULL;
		for (int i=0; i<emus->count; i++) {
			Entry* entry = emus->items[i];
			if (prev_entry!=NULL) {
				if (exactMatch(prev_entry->name, entry->name)) {
					Entry_free(entry);
					continue;
				}
			}
			Array_push(entries, entry);
			prev_entry = entry;
		}
		Array_free(emus); // just free the array part, entries now owns emus entries
		closedir(dh);
	}
	
	if (hasCollections()) {
		if (entries->count) Array_push(root, Entry_new(Paths.collectionsDir, kEntryDir));
		else { // no visible systems, promote collections to root
			dh = opendir(Paths.collectionsDir);
			if (dh!=NULL) {
				struct dirent *dp;
				char* tmp;
				char full_path[256];
				sprintf(full_path, "%s/", Paths.collectionsDir);
				tmp = full_path + strlen(full_path);
				Array* collections = Array_new();
				while((dp = readdir(dh)) != NULL) {
					if (hide(dp->d_name)) continue;
					strcpy(tmp, dp->d_name);
					Array_push(collections, Entry_new(full_path, kEntryDir)); // yes, collections are fake directories
				}
				EntryArray_sort(collections);
				for (int i=0; i<collections->count; i++) {
					Array_push(entries, collections->items[i]);
				}
				Array_free(collections); // just free the array part, entries now owns collections entries
				closedir(dh);
			}
		}
	}
	
	// add systems to root
	for (int i=0; i<entries->count; i++) {
		Array_push(root, entries->items[i]);
	}
	Array_free(entries); // root now owns entries' entries
	
	char tools_path[256];
	sprintf(tools_path, "%s/Tools", Paths.rootDir);
	if (!is_simple && exists(tools_path)) Array_push(root, Entry_new(tools_path, kEntryDir));
	
	return root;
}
static Array* getRecents(void) {
	Array* entries = Array_new();
	for (int i=0; i<recents->count; i++) {
		Recent* recent = recents->items[i];
		if (!recent->available) continue;
		
		char sd_path[256];
		sprintf(sd_path, "%s%s", Paths.rootDir, recent->path);
		int type = suffixMatch(".pak", sd_path) ? kEntryPak : kEntryRom; // ???
		Array_push(entries, Entry_new(sd_path, type));
	}
	return entries;
}
static Array* getCollection(char* path) {
	Array* entries = Array_new();
	FILE* file = fopen(path, "r");
	if (file) {
		char line[256];
		while (fgets(line,256,file)!=NULL) {
			normalizeNewline(line);
			trimTrailingNewlines(line);
			if (strlen(line)==0) continue; // skip empty lines
			
			char sd_path[256];
			sprintf(sd_path, "%s%s", Paths.rootDir, line);
			if (exists(sd_path)) {
				int type = suffixMatch(".pak", sd_path) ? kEntryPak : kEntryRom; // ???
				Array_push(entries, Entry_new(sd_path, type));
				
				// char emu_name[256];
				// getEmuName(sd_path, emu_name);
				// if (hasEmu(emu_name)) {
					// Array_push(entries, Entry_new(sd_path, kEntryRom));
				// }
			}
		}
		fclose(file);
	}
	return entries;
}
static Array* getDiscs(char* path){
	
	// TODO: does path have Paths.rootDir prefix?
	
	Array* entries = Array_new();
	
	char base_path[256];
	strcpy(base_path, path);
	char* tmp = strrchr(base_path, '/') + 1;
	tmp[0] = '\0';
	
	// TODO: limit number of discs supported (to 9?)
	FILE* file = fopen(path, "r");
	if (file) {
		char line[256];
		int disc = 0;
		while (fgets(line,256,file)!=NULL) {
			normalizeNewline(line);
			trimTrailingNewlines(line);
			if (strlen(line)==0) continue; // skip empty lines
			
			char disc_path[256];
			sprintf(disc_path, "%s%s", base_path, line);
						
			if (exists(disc_path)) {
				disc += 1;
				Entry* entry = Entry_new(disc_path, kEntryRom);
				free(entry->name);
				char name[16];
				sprintf(name, "Disc %i", disc);
				entry->name = strdup(name);
				Array_push(entries, entry);
			}
		}
		fclose(file);
	}
	return entries;
}
static int getFirstDisc(char* m3u_path, char* disc_path) { // based on getDiscs() natch
	int found = 0;

	char base_path[256];
	strcpy(base_path, m3u_path);
	char* tmp = strrchr(base_path, '/') + 1;
	tmp[0] = '\0';
	
	FILE* file = fopen(m3u_path, "r");
	if (file) {
		char line[256];
		while (fgets(line,256,file)!=NULL) {
			normalizeNewline(line);
			trimTrailingNewlines(line);
			if (strlen(line)==0) continue; // skip empty lines
			
			sprintf(disc_path, "%s%s", base_path, line);
						
			if (exists(disc_path)) found = 1;
			break;
		}
		fclose(file);
	}
	return found;
}

static void addEntries(Array* entries, char* path) {
	DIR *dh = opendir(path);
	if (dh!=NULL) {
		struct dirent *dp;
		char* tmp;
		char full_path[256];
		sprintf(full_path, "%s/", path);
		tmp = full_path + strlen(full_path);
		while((dp = readdir(dh)) != NULL) {
			if (hide(dp->d_name)) continue;
			strcpy(tmp, dp->d_name);
			int is_dir = dp->d_type==DT_DIR;
			int type;
			if (is_dir) {
				// TODO: this should make sure launch.sh exists
				if (suffixMatch(".pak", dp->d_name)) {
					type = kEntryPak;
				}
				else {
					type = kEntryDir;
				}
			}
			else {
				if (prefixMatch(Paths.collectionsDir, full_path)) {
					type = kEntryDir; // :shrug:
				}
				else {
					type = kEntryRom;
				}
			}
			Array_push(entries, Entry_new(full_path, type));
		}
		closedir(dh);
	}
}

static int isConsoleDir(char* path) {
	char* tmp;
	char parent_dir[256];
	strcpy(parent_dir, path);
	tmp = strrchr(parent_dir, '/');
	tmp[0] = '\0';
	
	return exactMatch(parent_dir, Paths.romsDir);
}

static Array* getEntries(char* path){
	Array* entries = Array_new();

	if (isConsoleDir(path)) { // top-level console folder, might collate
		char collated_path[256];
		strcpy(collated_path, path);
		char* tmp = strrchr(collated_path, '(');
		if (tmp) {
			tmp[1] = '\0'; // 1 because we want to keep the opening parenthesis to avoid collating "Game Boy Color" and "Game Boy Advance" into "Game Boy"

			DIR *dh = opendir(Paths.romsDir);
			if (dh!=NULL) {
				struct dirent *dp;
				char full_path[256];
				sprintf(full_path, "%s/", Paths.romsDir);
				tmp = full_path + strlen(full_path);
				// while loop so we can collate paths, see above
				while((dp = readdir(dh)) != NULL) {
					if (hide(dp->d_name)) continue;
					if (dp->d_type!=DT_DIR) continue;
					strcpy(tmp, dp->d_name);
				
					if (!prefixMatch(collated_path, full_path)) continue;
					addEntries(entries, full_path);
				}
				closedir(dh);
			}
		}
	}
	else addEntries(entries, path); // just a subfolder
	
	EntryArray_sort(entries);
	return entries;
}

///////////////////////////////////////

static void queueNext(char* cmd) {
	putFile("/tmp/next", cmd);
	quit = 1;
}
static char* escapeSingleQuotes(char* str) {
	// based on https://stackoverflow.com/a/31775567/145965
	int replaceString(char *line, const char *search, const char *replace) {
	   char *sp; // start of pattern
	   if ((sp = strstr(line, search)) == NULL) {
	      return 0;
	   }
	   int count = 1;
	   int sLen = strlen(search);
	   int rLen = strlen(replace);
	   if (sLen > rLen) {
	      // move from right to left
	      char *src = sp + sLen;
	      char *dst = sp + rLen;
	      while((*dst = *src) != '\0') { dst++; src++; }
	   } else if (sLen < rLen) {
	      // move from left to right
	      int tLen = strlen(sp) - sLen;
	      char *stop = sp + rLen;
	      char *src = sp + sLen + tLen;
	      char *dst = sp + rLen + tLen;
	      while(dst >= stop) { *dst = *src; dst--; src--; }
	   }
	   memcpy(sp, replace, rLen);
	   count += replaceString(sp + rLen, search, replace);
	   return count;
	}
	replaceString(str, "'", "'\\''");
	return str;
}

///////////////////////////////////////

static void readyResumePath(char* rom_path, int type) {
	char* tmp;
	can_resume = 0;
	char path[256];
	strcpy(path, rom_path);
	
	if (!prefixMatch(Paths.romsDir, path)) return;
	
	char auto_path[256];
	if (type==kEntryDir) {
		if (!hasCue(path, auto_path)) { // no cue?
			tmp = strrchr(auto_path, '.') + 1; // extension
			strcpy(tmp, "m3u"); // replace with m3u
			if (!exists(auto_path)) return; // no m3u
		}
		strcpy(path, auto_path); // cue or m3u if one exists
	}
	
	if (!suffixMatch(".m3u", path)) {
		char m3u_path[256];
		if (hasM3u(path, m3u_path)) {
			// change path to m3u path
			strcpy(path, m3u_path);
		}
	}
	
	char emu_name[256];
	getEmuName(path, emu_name);
	
	char rom_file[256];
	tmp = strrchr(path, '/') + 1;
	strcpy(rom_file, tmp);
	
	sprintf(slot_path, "%s/.mmenu/%s/%s.txt", Paths.userdataDir, emu_name, rom_file); // /.userdata/.mmenu/<EMU>/<romname>.ext.txt
	
	can_resume = exists(slot_path);
}
static void readyResume(Entry* entry) {
	readyResumePath(entry->path, entry->type);
}

static void saveLast(char* path);
static void loadLast(void);

static int autoResume(void) {
	// NOTE: bypasses recents

	if (!exists(kAutoResumePath)) return 0;
	
	char path[256];
	getFile(kAutoResumePath, path, 256);
	unlink(kAutoResumePath);
	sync();
	
	// make sure rom still exists
	char sd_path[256];
	sprintf(sd_path, "%s%s", Paths.rootDir, path);
	if (!exists(sd_path)) return 0;
	
	// make sure emu still exists
	char emu_name[256];
	getEmuName(sd_path, emu_name);
	
	char emu_path[256];
	getEmuPath(emu_name, emu_path);
	
	if (!exists(emu_path)) return 0;
	
	char cmd[256];
	sprintf(cmd, "'%s' '%s'", escapeSingleQuotes(emu_path), escapeSingleQuotes(sd_path));
	putFile(kResumeSlotPath, kAutoResumeSlot);

	// putFile(kLastPath, Paths.fauxRecentDir); // saveLast() will crash here because top is NULL
	queueNext(cmd);
	return 1;
}

static void openPak(char* path) {
	char cmd[256];
	sprintf(cmd, "'%s/launch.sh'", escapeSingleQuotes(path));
	
	// if (prefixMatch(Paths.romsDir, path)) {
	// 	addRecent(path);
	// }
	saveLast(path);
	queueNext(cmd);
}
static void openRom(char* path, char* last) {
	char sd_path[256];
	strcpy(sd_path, path);
	
	char m3u_path[256];
	int has_m3u = hasM3u(sd_path, m3u_path);
	
	char recent_path[256];
	strcpy(recent_path, has_m3u ? m3u_path : sd_path);
	
	if (has_m3u && suffixMatch(".m3u", sd_path)) {
		getFirstDisc(m3u_path, sd_path);
	}

	char emu_name[256];
	getEmuName(sd_path, emu_name);

	if (should_resume) {
		char slot[16];
		getFile(slot_path, slot, 16);
		putFile(kResumeSlotPath, slot);
		should_resume = 0;

		if (has_m3u) {
			char rom_file[256];
			strcpy(rom_file, strrchr(m3u_path, '/') + 1);
			
			// get disc for state
			char disc_path_path[256];
			sprintf(disc_path_path, "%s/.mmenu/%s/%s.%s.txt", Paths.userdataDir, emu_name, rom_file, slot); // /.userdata/.mmenu/<EMU>/<romname>.ext.0.txt

			if (exists(disc_path_path)) {
				// switch to disc path
				char disc_path[256];
				getFile(disc_path_path, disc_path, 256);
				if (disc_path[0]=='/') strcpy(sd_path, disc_path); // absolute
				else { // relative
					strcpy(sd_path, m3u_path);
					char* tmp = strrchr(sd_path, '/') + 1;
					strcpy(tmp, disc_path);
				}
			}
		}
	}
	else putInt(kResumeSlotPath,8); // resume hidden default state
	
	char emu_path[256];
	getEmuPath(emu_name, emu_path);
	
	char cmd[256];
	sprintf(cmd, "'%s' '%s'", escapeSingleQuotes(emu_path), escapeSingleQuotes(sd_path));

	addRecent(recent_path);
	saveLast(last==NULL ? sd_path : last);
	queueNext(cmd);
}
static void openDirectory(char* path, int auto_launch) {
	char auto_path[256];
	if (hasCue(path, auto_path) && auto_launch) {
		openRom(auto_path, path);
		return;
	}

	char m3u_path[256];
	strcpy(m3u_path, auto_path);
	char* tmp = strrchr(m3u_path, '.') + 1; // extension
	strcpy(tmp, "m3u"); // replace with m3u
	if (exists(m3u_path) && auto_launch) {
		auto_path[0] = '\0';
		if (getFirstDisc(m3u_path, auto_path)) {
			openRom(auto_path, path);
			return;
		}
		// TODO: doesn't handle empty m3u files
	}
	
	int selected = 0;
	int start = selected;
	int end = 0;
	if (top && top->entries->count>0) {
		if (restore_depth==stack->count && top->selected==restore_relative) {
			selected = restore_selected;
			start = restore_start;
			end = restore_end;
		}
	}
	
	top = Directory_new(path, selected);
	top->start = start;
	top->end = end ? end : ((top->entries->count<Screen.main.list.row_count) ? top->entries->count : Screen.main.list.row_count);

	Array_push(stack, top);
}
static void closeDirectory(void) {
	restore_selected = top->selected;
	restore_start = top->start;
	restore_end = top->end;
	DirectoryArray_pop(stack);
	restore_depth = stack->count;
	top = stack->items[stack->count-1];
	restore_relative = top->selected;
}

static void Entry_open(Entry* self) {
	if (self->type==kEntryRom) {
		char *last = NULL;
		if (prefixMatch(Paths.collectionsDir, top->path)) {
			char* tmp;
			char filename[256];
			
			tmp = strrchr(self->path, '/');
			if (tmp) strcpy(filename, tmp+1);
			
			char last_path[256];
			sprintf(last_path, "%s/%s", top->path, filename);
			last = last_path;
		}
		openRom(self->path, last);
	}
	else if (self->type==kEntryPak) {
		openPak(self->path);
	}
	else if (self->type==kEntryDir) {
		openDirectory(self->path, 1);
	}
}

///////////////////////////////////////

static void saveLast(char* path) {
	// special case for recently played
	if (exactMatch(top->path, Paths.fauxRecentDir)) {
		// NOTE: that we don't have to save the file because
		// your most recently played game will always be at
		// the top which is also the default selection
		path = Paths.fauxRecentDir;
	}
	putFile(kLastPath, path);
}
static void loadLast(void) { // call after loading root directory
	if (!exists(kLastPath)) return;

	char last_path[256];
	getFile(kLastPath, last_path, 256);
	
	char full_path[256];
	strcpy(full_path, last_path);
	
	char* tmp;
	char filename[256];
	tmp = strrchr(last_path, '/');
	if (tmp) strcpy(filename, tmp);
	
	Array* last = Array_new();
	while (!exactMatch(last_path, Paths.rootDir)) {
		Array_push(last, strdup(last_path));
		
		char* slash = strrchr(last_path, '/');
		last_path[(slash-last_path)] = '\0';
	}
	
	while (last->count>0) {
		char* path = Array_pop(last);
		if (!exactMatch(path, Paths.romsDir)) { // romsDir is effectively root as far as restoring state after a game
			char collated_path[256];
			collated_path[0] = '\0';
			if (suffixMatch(")", path) && isConsoleDir(path)) {
				strcpy(collated_path, path);
				tmp = strrchr(collated_path, '(');
				if (tmp) tmp[1] = '\0'; // 1 because we want to keep the opening parenthesis to avoid collating "Game Boy Color" and "Game Boy Advance" into "Game Boy"
			}
			
			for (int i=0; i<top->entries->count; i++) {
				Entry* entry = top->entries->items[i];
			
				// NOTE: strlen() is required for collated_path, '\0' wasn't reading as NULL for some reason
				if (exactMatch(entry->path, path) || (strlen(collated_path) && prefixMatch(collated_path, entry->path)) || (prefixMatch(Paths.collectionsDir, full_path) && suffixMatch(filename, entry->path))) {
					top->selected = i;
					if (i>=top->end) {
						top->start = i;
						top->end = top->start + Screen.main.list.row_count;
						if (top->end>top->entries->count) {
							top->end = top->entries->count;
							top->start = top->end - Screen.main.list.row_count;
						}
					}
					if (last->count==0 && !exactMatch(entry->path, Paths.fauxRecentDir) && !(!exactMatch(entry->path, Paths.collectionsDir) && prefixMatch(Paths.collectionsDir, entry->path))) break; // don't show contents of auto-launch dirs
				
					if (entry->type==kEntryDir) {
						openDirectory(entry->path, 0);
						break;
					}
				}
			}
		}
		free(path); // we took ownership when we popped it
	}
	
	StringArray_free(last);
}

///////////////////////////////////////

static void Menu_init(void) {
	stack = Array_new(); // array of open Directories
	recents = Array_new();

	openDirectory(Paths.rootDir, 0);
	loadLast(); // restore state when available
}
static void Menu_quit(void) {
	RecentArray_free(recents);
	DirectoryArray_free(stack);
}

///////////////////////////////////////

int main (int argc, char *argv[]) {
	if (autoResume()) return 0; // nothing to do
	
	is_simple = exists(kEnableSimpleModePath);
	
	dump("MiniUI");
	
	putenv("SDL_HIDE_BATTERY=1");
	
	SDL_Init(SDL_INIT_VIDEO);
	TTF_Init();
	
	SDL_ShowCursor(0);
	SDL_EnableKeyRepeat(300,100);
	
	InitSettings();
	
	SDL_Surface* screen = SDL_SetVideoMode(Screen.width, Screen.height, 16, SDL_HWSURFACE | SDL_DOUBLEBUF);
	
	GFX_init();
	GFX_ready();
	
	SDL_Surface* logo = GFX_loadImage("logo.png");
	SDL_Surface* version = NULL;
	
	Menu_init();
	
	Input_reset();
	int dirty = 1;
	int was_charging = isCharging();
	unsigned long charge_start = SDL_GetTicks();
	int show_version = 0;
	int show_setting = 0; // 1=brightness,2=volume
	int setting_value = 0;
	int setting_min = 0;
	int setting_max = 0;
	int delay_start = 0;
	int delay_select = 0;
	unsigned long cancel_start = SDL_GetTicks();
	unsigned long power_start = 0;
	while (!quit) {
		unsigned long frame_start = SDL_GetTicks();
		
		Input_poll();
			
		int selected = top->selected;
		int total = top->entries->count;
		
		if (show_version) {
			if (Input_justPressed(kButtonB) || Input_justPressed(kButtonMenu)) {
				show_version = 0;
				dirty = 1;
			}
		}
		else {
			if (!is_simple && Input_justPressed(kButtonMenu)) {
				show_version = 1;
				dirty = 1;
			}
			
			if (total>0) {
				if (Input_justRepeated(kButtonUp)) {
					selected -= 1;
					if (selected<0) {
						selected = total-1;
						int start = total - Screen.main.list.row_count;
						top->start = (start<0) ? 0 : start;
						top->end = total;
					}
					else if (selected<top->start) {
						top->start -= 1;
						top->end -= 1;
					}
				}
				else if (Input_justRepeated(kButtonDown)) {
					selected += 1;
					if (selected>=total) {
						selected = 0;
						top->start = 0;
						top->end = (total<Screen.main.list.row_count) ? total : Screen.main.list.row_count;
					}
					else if (selected>=top->end) {
						top->start += 1;
						top->end += 1;
					}
				}
				if (Input_justRepeated(kButtonLeft)) {
					selected -= Screen.main.list.row_count;
					if (selected<0) {
						selected = 0;
						top->start = 0;
						top->end = (total<Screen.main.list.row_count) ? total : Screen.main.list.row_count;
					}
					else if (selected<top->start) {
						top->start -= Screen.main.list.row_count;
						if (top->start<0) top->start = 0;
						top->end = top->start + Screen.main.list.row_count;
					}
				}
				else if (Input_justRepeated(kButtonRight)) {
					selected += Screen.main.list.row_count;
					if (selected>=total) {
						selected = total-1;
						int start = total - Screen.main.list.row_count;
						top->start = (start<0) ? 0 : start;
						top->end = total;
					}
					else if (selected>=top->end) {
						top->end += Screen.main.list.row_count;
						if (top->end>total) top->end = total;
						top->start = top->end - Screen.main.list.row_count;
					}
				}
			}
			
			if (!Input_isPressed(kButtonStart) && !Input_isPressed(kButtonSelect)) { 
				if (Input_justRepeated(kButtonL)) { // previous alpha
					Entry* entry = top->entries->items[selected];
					int i = entry->alpha-1;
					if (i>=0) {
						selected = top->alphas->items[i];
						if (total>Screen.main.list.row_count) {
							top->start = selected;
							top->end = top->start + Screen.main.list.row_count;
							if (top->end>total) top->end = total;
							top->start = top->end - Screen.main.list.row_count;
						}
					}
				}
				else if (Input_justRepeated(kButtonR)) { // next alpha
					Entry* entry = top->entries->items[selected];
					int i = entry->alpha+1;
					if (i<top->alphas->count) {
						selected = top->alphas->items[i];
						if (total>Screen.main.list.row_count) {
							top->start = selected;
							top->end = top->start + Screen.main.list.row_count;
							if (top->end>total) top->end = total;
							top->start = top->end - Screen.main.list.row_count;
						}
					}
				}
			}
		
			if (selected!=top->selected) {
				top->selected = selected;
				dirty = 1;
			}
		
			if (dirty && total>0) readyResume(top->entries->items[top->selected]);

			if (total>0 && Input_justReleased(kButtonResume)) {
				if (can_resume) {
					should_resume = 1;
					Entry_open(top->entries->items[top->selected]);
					dirty = 1;
				}
			}
			else if (total>0 && Input_justPressed(kButtonA)) {
				Entry_open(top->entries->items[top->selected]);
				total = top->entries->count;
				dirty = 1;
	
				if (total>0) readyResume(top->entries->items[top->selected]);
			}
			else if (Input_justPressed(kButtonB) && stack->count>1) {
				closeDirectory();
				total = top->entries->count;
				dirty = 1;
				// can_resume = 0;
				if (total>0) readyResume(top->entries->items[top->selected]);
			}
		}
		
		unsigned long now = SDL_GetTicks();
		if (Input_anyPressed()) cancel_start = now;
		
		#define kChargeDelay 1000
		if (dirty || now-charge_start>=kChargeDelay) {
			int is_charging = isCharging();
			if (was_charging!=is_charging) {
				was_charging = is_charging;
				dirty = 1;
			}
			charge_start = now;
		}
		
		if (power_start && now-power_start>=1000) {
			powerOff();
			// return 0; // we should never reach this point
		}
		if (Input_justPressed(kButtonSleep)) {
			power_start = now;
		}
		
		#define kSleepDelay 30000
		if (now-cancel_start>=kSleepDelay && preventAutosleep()) cancel_start = now;
		
		if (now-cancel_start>=kSleepDelay || Input_justReleased(kButtonSleep)) // || Input_justPressed(kButtonMenu))
		{
			fauxSleep();
			cancel_start = SDL_GetTicks();
			power_start = 0;
			dirty = 1;
		}
		
		int was_dirty = dirty; // dirty list (not including settings/battery)
		
		int old_setting = show_setting;
		int old_value = setting_value;
		show_setting = 0;
		if (Input_isPressed(kButtonStart) && Input_isPressed(kButtonSelect)) {
			// buh
		}
		else if (Input_isPressed(kButtonStart) && !delay_start) {
			show_setting = 1;
			setting_value = GetBrightness();
			setting_min = MIN_BRIGHTNESS;
			setting_max = MAX_BRIGHTNESS;
		}
		else if (Input_isPressed(kButtonSelect) && !delay_select) {
			show_setting = 2;
			setting_value = GetVolume();
			setting_min = MIN_VOLUME;
			setting_max = MAX_VOLUME;
		}
		if (old_setting!=show_setting || old_value!=setting_value) dirty = 1;
		
		if (dirty) {
			SDL_FillRect(screen, NULL, 0);
			SDL_BlitSurface(logo, NULL, screen, &(SDL_Rect){Screen.main.logo.x,Screen.main.logo.y});

			if (show_setting) {
				GFX_blitSettings(screen, Screen.main.settings.x, Screen.main.settings.y, show_setting==1?0:(setting_value>0?1:2), setting_value,setting_min,setting_max);
			}
			else {
				GFX_blitBattery(screen, Screen.main.battery.x, Screen.main.battery.y);
			}
			GFX_blitRule(screen, Screen.main.rule.top_y);
		
			if (show_version) {
				if (!version) {
					char release[256];
					getFile("./version.txt", release, 256);
					
					char *tmp,*commit;
					commit = strrchr(release, '\n');
					commit[0] = '\0';
					commit = strrchr(release, '\n')+1;
					tmp = strchr(release, '\n');
					tmp[0] = '\0';
					
					SDL_Surface* release_txt = GFX_getText("Release", 2, 1);
					SDL_Surface* version_txt = GFX_getText(release, 2, 0);
					SDL_Surface* commit_txt = GFX_getText("Commit", 2, 1);
					SDL_Surface* hash_txt = GFX_getText(commit, 2, 0);
					
					SDL_Surface* firmware_txt = GFX_getText("Firmware", 2, 1);
					SDL_Surface* date_txt = GFX_getText(getenv("MIYOO_VERSION"), 2, 0);
				
					int x = firmware_txt->w + 12;
					int w = x + version_txt->w;
					int h = 96 * 2;
					version = SDL_CreateRGBSurface(0,w,h,16,0,0,0,0);
				
					SDL_BlitSurface(release_txt, NULL, version, &(SDL_Rect){0, 0});
					SDL_BlitSurface(version_txt, NULL, version, &(SDL_Rect){x,0});
					SDL_BlitSurface(commit_txt, NULL, version, &(SDL_Rect){0,48});
					SDL_BlitSurface(hash_txt, NULL, version, &(SDL_Rect){x,48});
					SDL_BlitSurface(firmware_txt, NULL, version, &(SDL_Rect){0,144});
					SDL_BlitSurface(date_txt, NULL, version, &(SDL_Rect){x,144});
	
					SDL_FreeSurface(release_txt);
					SDL_FreeSurface(version_txt);
					SDL_FreeSurface(commit_txt);
					SDL_FreeSurface(hash_txt);
					SDL_FreeSurface(firmware_txt);
					SDL_FreeSurface(date_txt);
				}
				SDL_BlitSurface(version, NULL, screen, &(SDL_Rect){(640-version->w)/2,(480-version->h)/2});
			}
			else {
				if (total>0) {
					int selected_row = top->selected - top->start;
					for (int i=top->start,j=0; i<top->end; i++,j++) {
						Entry* entry = top->entries->items[i];
						GFX_blitMenu(screen, entry->name, entry->path, entry->unique, j, selected_row);
					}
				}
				else {
					GFX_blitBodyCopy(screen, "Empty folder", 0,0,Screen.width,Screen.height);
				}
			}
		
			GFX_blitRule(screen, Screen.main.rule.bottom_y);
			if (can_resume && !show_version) {
				if (strlen(HINT_RESUME)>1) GFX_blitPill(screen, HINT_RESUME, "RESUME", Screen.buttons.left, Screen.buttons.top);
				else GFX_blitButton(screen, HINT_RESUME, "RESUME", Screen.buttons.left, Screen.buttons.top, Screen.button.text.ox_X);
			}
			else {
				GFX_blitPill(screen, HINT_SLEEP, "SLEEP", Screen.buttons.left, Screen.buttons.top);
			}
			
			if (show_version) {
				GFX_blitButton(screen, "B", "BACK", -Screen.buttons.right, Screen.buttons.top, Screen.button.text.ox_B);
			}
			else if (total==0) {
				if (stack->count>1) {
					GFX_blitButton(screen, "B", "BACK", -Screen.buttons.right, Screen.buttons.top, Screen.button.text.ox_B);
				}
			}
			else {
				int button_width = GFX_blitButton(screen, "A", "OPEN", -Screen.buttons.right, Screen.buttons.top, Screen.button.text.ox_A);
				if (stack->count>1) {
					GFX_blitButton(screen, "B", "BACK", -(Screen.buttons.right+button_width+Screen.buttons.gutter),Screen.buttons.top, Screen.button.text.ox_B);
				}
			}
		}
		
		// scroll long names
		if (total>0) {
			int selected_row = top->selected - top->start;
			Entry* entry = top->entries->items[top->selected];
			if (GFX_scrollMenu(screen, entry->name, entry->path, entry->unique, selected_row, top->selected, was_dirty, dirty)) dirty = 1;
		}
		
		if (dirty) {
			SDL_Flip(screen);
			dirty = 0;
		}		
		// slow down to 60fps
		unsigned long frame_duration = SDL_GetTicks() - frame_start;
		#define kTargetFrameDuration 17
		if (frame_duration<kTargetFrameDuration) SDL_Delay(kTargetFrameDuration-frame_duration);
	}
	
	SDL_FillRect(screen, NULL, 0);
	SDL_Flip(screen);
	
	SDL_FreeSurface(logo);
	if (version) SDL_FreeSurface(version);

	Menu_quit();
	GFX_quit();

	SDL_Quit();
	QuitSettings();
}