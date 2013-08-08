#include <tomato.h>
struct translation {
	char *name;
	char *value;
	struct translation *next;
};
static struct translation *trans[] = {0,0,0};
int load_dictionary(){
	char *langs[] = {"en-us.dict",0,0};
	char line[1024];
	struct translation *current;
	int i,j;
	char *dictionary_file = nvram_get("default_web_lang_file");
	if(dictionary_file && *dictionary_file)
		langs[1] = dictionary_file;
	for(i=0 ; i < sizeof(langs) / sizeof(char *); i++) {
		if(!langs[i])
			continue;
		FILE *fp = fopen (langs[i], "rb");
		if(fp){
			current = NULL;
			while(fgets(line,sizeof(line),fp) != NULL){
				for(j = strlen(line) - 1 ; j >=0 ;j--)
				{
					if(line[j] == '\r' || line[j] == '\n'){
						line[j] = 0;
					}
					else
						break;
				}
				char *equal = strstr(line,"=");
				if(equal){
					struct translation *tmp = calloc(1,sizeof(struct translation));
					tmp-> name = calloc(1,equal - line + 1);
					tmp-> value = calloc(1,line + strlen(line) - equal);
					memcpy(tmp->name,line,equal - line);
					memcpy(tmp->value,equal + 1, line + strlen(line) - equal - 1);
					if(trans[i] == NULL)
						trans[i] = tmp;
					else if (current){
						current->next = tmp;
					}
					current = tmp;
				}
			}
			fclose(fp);
		}
	}
	return 1;
}
void release_dictionary(){
	struct translation *tmp;
	int i;
	for(i = 0 ; i < sizeof(trans) / sizeof(struct translation *);i++){
		while(trans[i]) {
			free(trans[i]->name);
			free(trans[i]->value);
			tmp = trans[i]->next;
			free(trans[i]);
			trans[i] = tmp;
		}
	}
}
static char* do_translate(char* start,char *end){
	int i;
	for(i = sizeof(trans) / sizeof(struct translation *) - 1; i>=0 ; i--){
		struct translation *tmp = trans[i];		
		while(tmp) {
			if(end - start == strlen(tmp->name) && !memcmp(tmp->name,start,end - start )){
				return tmp->value;
			}
			tmp = tmp->next;
		}
	}
	return NULL;
}
int translate(char *buffer, char **replaced,int max){
	char *end;
	char *start = buffer;
	*replaced = calloc(1,max);
	while((start = strstr(buffer,"<#"))) {
		sprintf(*replaced + strlen(*replaced),"%.*s",start - buffer, buffer);
		end = strstr(start,"#>");
		if(end){
			strcat(*replaced,do_translate(start + 2,end)?:"");
			buffer = end + 2;
		}
		else {
			strcat(*replaced,start);
			return 0;
		}
	}
	if(strlen(buffer) > 0)
		strcat(*replaced,buffer);
	return 0;	
}
