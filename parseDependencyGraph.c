/* The following program is 
used to parse dependency 
trees based page load process 
in todays browser

written by-- Mohd Rajiullah*/

 
#include <stdio.h>
#include <stdlib.h>
#include "cJSON.h"

/* Parse text to JSON, then render back to text, and print! */
void doit(char *text)
{
   cJSON *temp_obj;
   cJSON *json=cJSON_Parse(text);
   if (!json) {printf("Error before, 1: [%s]\n",cJSON_GetErrorPtr());}


   cJSON *objs=cJSON_GetObjectItem(json,"objs");
   if (!objs) {printf("Error before, 2: [%s]\n",cJSON_GetErrorPtr());}
   
   cJSON *deps=cJSON_GetObjectItem(json,"deps");
   if (!deps) {printf("Error before, 3: [%s]\n",cJSON_GetErrorPtr());}
   
   int i,j;
   char *out, *a1, *a2;
   
   cJSON *this_objs_array=cJSON_CreateArray();
   cJSON *this_acts_array=cJSON_CreateArray();
   cJSON *this_objs, *temp1, *temp2, *this_acts, *comps, *root, *b1, *b2, *temp, *temp_array;
   
   for(i=0; i<cJSON_GetArraySize(objs); i++)
		{
		cJSON * obj= cJSON_GetArrayItem(objs, i);
		/* Handle array of comp */
	        comps=cJSON_GetObjectItem(obj,"comps");
      		if (!comps) {printf("Error before, 4: [%s]\n",cJSON_GetErrorPtr());}
		
		/* Handle download */
		cJSON * download=cJSON_GetObjectItem(obj,"download");
        	cJSON_AddStringToObject(download, "obj_id",cJSON_GetObjectItem(obj,"id")->valuestring);
        	cJSON_AddStringToObject(download, "mime","download");
		
		/* Index obj by object id */
		
		/*printf("||||||\n");
		out=cJSON_Print(obj);
		printf("%s\n",out);
		printf("||||||\n");*/
		
		temp1=cJSON_CreateObject();
		cJSON_AddStringToObject(temp1,"id",cJSON_GetObjectItem(obj,"id")->valuestring);
		cJSON_AddStringToObject(temp1,"host",cJSON_GetObjectItem(obj,"host")->valuestring);
		cJSON_AddStringToObject(temp1,"path",cJSON_GetObjectItem(obj,"path")->valuestring);
		cJSON_AddNumberToObject(temp1,"when_comp_start",cJSON_GetObjectItem(obj,"when_comp_start")->valueint);
		cJSON_AddItemReferenceToObject(temp1,"download",download);
		cJSON_AddItemReferenceToObject(temp1,"comps",comps);

		this_objs=cJSON_CreateObject();
		
		cJSON_AddItemReferenceToObject(this_objs,cJSON_GetObjectItem(obj,"id")->valuestring,temp1);
		cJSON_AddItemReferenceToArray(this_objs_array,this_objs);
		
		/*temp2=cJSON_CreateObject();
	
		cJSON_AddStringToObject(temp2,"id",cJSON_GetObjectItem(download,"id")->valuestring);
		cJSON_AddStringToObject(temp2,"type",cJSON_GetObjectItem(download,"type")->valuestring);
		cJSON_AddStringToObject(temp2,"obj_id",cJSON_GetObjectItem(download,"obj_id")->valuestring);
		cJSON_AddStringToObject(temp2,"mime",cJSON_GetObjectItem(download,"mime")->valuestring);
		this_acts=cJSON_CreateObject();
                		
		cJSON_AddItemReferenceToObject(this_acts,cJSON_GetObjectItem(download,"id")->valuestring,temp2);*/
		this_acts=cJSON_CreateObject();
		cJSON_AddItemReferenceToObject(this_acts,cJSON_GetObjectItem(download,"id")->valuestring,download);
		cJSON_AddItemReferenceToArray(this_acts_array,this_acts);



	
		

		 for (j=0; j< cJSON_GetArraySize(comps); j++)
			{
			cJSON * comp= cJSON_GetArrayItem(comps, j);
			cJSON_AddStringToObject(comp, "obj_id",cJSON_GetObjectItem(obj,"id")->valuestring);
			 /*Add natural dependency
      			   Depends on download activity
      			   Depends on its previous comp activity*/
		 	 if (j==0){
				a1=cJSON_GetObjectItem(download,"id")->valuestring;
        		  }
			else{
				a1=cJSON_GetObjectItem(cJSON_GetArrayItem(comps, j-1),"id")->valuestring;
			}
			 root=cJSON_CreateObject();
        		 int deps_length=cJSON_GetArraySize(deps);
			 char dep_id[6];
			 sprintf(dep_id,"dep%d",deps_length+1);
        		 cJSON_AddStringToObject(root,"id",dep_id);	
        		 cJSON_AddStringToObject(root,"a1",a1);	
        		 cJSON_AddStringToObject(root,"a2",cJSON_GetObjectItem(comp,"id")-> valuestring);	
			 cJSON_AddNumberToObject(root,"time",-1);
			 cJSON_AddItemReferenceToArray(deps,root);
			 //out=cJSON_Print(deps);
			 //printf("%s\n",out);
			 temp2=cJSON_CreateObject();
	
			 cJSON_AddStringToObject(temp2,"id",cJSON_GetObjectItem(comp,"id")->valuestring);
			 cJSON_AddStringToObject(temp2,"type",cJSON_GetObjectItem(comp,"type")->valuestring);
			 cJSON_AddNumberToObject(temp2,"time",cJSON_GetObjectItem(comp,"time")->valueint);
			 cJSON_AddStringToObject(temp2,"obj_id",cJSON_GetObjectItem(obj,"id")->valuestring);
			 this_acts=cJSON_CreateObject();
                		
		         cJSON_AddItemReferenceToObject(this_acts,cJSON_GetObjectItem(comp,"id")->valuestring,temp2);
		         cJSON_AddItemReferenceToArray(this_acts_array,this_acts);

			
		 }
		
		}
     /*out=cJSON_Print(deps);
     printf("%s\n",out);*/
		
    /* Add dependencies to activities */
    for (i=0; i< cJSON_GetArraySize(deps); i++){
	cJSON * dep= cJSON_GetArrayItem(deps,i);
	
	//printf("==========");
	for (j=0; j< cJSON_GetArraySize(this_acts_array); j++){
		cJSON * obj= cJSON_GetArrayItem(this_acts_array, j);
		
		 out=cJSON_Print(obj);
	 	 char *Cid=malloc(10);
		 sprintf(Cid,"%.9s",out);
   		
		 char *pch = strstr(Cid, cJSON_GetObjectItem(dep,"a1")->valuestring);
		 if(pch){
			
			b1= cJSON_GetObjectItem(obj,cJSON_GetObjectItem(dep,"a1")->valuestring);
			//out=cJSON_Print(b1);
	 	 	//printf("%s\n",out);
			
		}
		
		char *pch1 = strstr(Cid, cJSON_GetObjectItem(dep,"a2")->valuestring);
		 if(pch1){
			b2= cJSON_GetObjectItem(obj,cJSON_GetObjectItem(dep,"a2")->valuestring);
			//out=cJSON_Print(b2);
	 	 	//printf("%s\n",out);
			}
		
	}
	//printf("==========");
	 
	
	/* Add to a2 that 'a2 depends on a1' */

	temp=cJSON_CreateObject();
	temp_array=cJSON_CreateArray();
	cJSON_AddStringToObject(temp,"id",cJSON_GetObjectItem(b1,"id")->valuestring);
	cJSON_AddNumberToObject(temp,"time",cJSON_GetObjectItem(dep,"time")->valueint);
	cJSON_AddItemToArray(temp_array,temp);
	cJSON_AddItemReferenceToObject(b2,"deps",temp_array);

	temp=cJSON_CreateObject();
	temp_array=cJSON_CreateArray();
	cJSON_AddStringToObject(temp,"id",cJSON_GetObjectItem(b2,"id")->valuestring);
	cJSON_AddNumberToObject(temp,"time",cJSON_GetObjectItem(dep,"time")->valueint);
	cJSON_AddItemToArray(temp_array,temp);
	cJSON_AddItemReferenceToObject(b1,"triggers",temp_array);
	
	/* out=cJSON_Print(b1);
	 printf("%s\n",out);
	 out=cJSON_Print(b2);
	 printf("%s\n",out);*/
	
	
    }

      printf("===[objects]");
	out=cJSON_Print(this_objs_array);
	//cJSON_Delete(this_objs);
	printf("%s\n",out);	
	printf("===[activities]");
	out=cJSON_Print(this_acts_array);
	//cJSON_Delete(this_acts);
	printf("%s\n",out);
	//free(out);
   
   
		
    /*for(i=0; i<cJSON_GetArraySize(this_acts_array); i++)
		{
			cJSON * obj= cJSON_GetArrayItem(this_acts_array, i);
			printf("......\n");
			out=cJSON_Print(obj);
			printf("%s\n",out);
		}*/
	
}



/* Read a file, parse, render back, etc. */
void dofile(char *filename)
{
	FILE *f;long len;char *data;
	
	f=fopen(filename,"rb");
	if (f == NULL)
   		 {
        	perror("Error opening file\n");
		exit(0);
   	}
	fseek(f,0,SEEK_END);len=ftell(f);fseek(f,0,SEEK_SET);
	data=(char*)malloc(len+1);fread(data,1,len,f);fclose(f);
	doit(data);
	free(data);
}


int main (int argc, char * argv[]) {
	
	if(argc !=2){
		printf("usage: %s filename\n",argv[0]);
		exit(0);
	}

	dofile(argv[1]); 

	
	return 0;
}
