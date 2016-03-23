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
   if (!json) {printf("Error before: [%s]\n",cJSON_GetErrorPtr());}


   cJSON *objs=cJSON_GetObjectItem(json,"objs");
   if (!objs) {printf("Error before: [%s]\n",cJSON_GetErrorPtr());}
   
   cJSON *deps=cJSON_GetObjectItem(json,"deps");
   if (!deps) {printf("Error before: [%s]\n",cJSON_GetErrorPtr());}
   
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
      		if (!comps) {printf("Error before: [%s]\n",cJSON_GetErrorPtr());}
		
		/* Handle download */
		cJSON * download=cJSON_GetObjectItem(obj,"download");
        	cJSON_AddStringToObject(download, "obj_id",cJSON_GetObjectItem(obj,"id")->valuestring);
        	cJSON_AddStringToObject(download, "mime","download");
		
		/* Index obj by object id */
		
		
		
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


	        
	
		/* printf("||||||\n");
		out=cJSON_Print(obj);
		printf("%s\n",out);
		printf("||||||\n"); */

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
			
			 temp2=cJSON_CreateObject();
		          
                         
	
			 cJSON_AddStringToObject(temp2,"id",cJSON_GetObjectItem(comp,"id")->valuestring);
			 
			
			// cJSON_AddStringToObject(temp2,"type",cJSON_GetObjectItem(comp,"type")->valuestring);			
			//printf ("==%s==\n",cJSON_GetObjectItem(comp,"type")->valuestring);
			if (cJSON_GetObjectItem(comp,"type")->valuestring!=NULL)
				cJSON_AddStringToObject(temp2,"type",cJSON_GetObjectItem(comp,"type")->valuestring);
			else
				cJSON_AddNumberToObject(temp2,"type",cJSON_GetObjectItem(comp,"type")->valueint);
			
			 //printf("%s\n",a1);
			
			 cJSON_AddNumberToObject(temp2,"time",cJSON_GetObjectItem(comp,"time")->valueint);
			 cJSON_AddStringToObject(temp2,"obj_id",cJSON_GetObjectItem(obj,"id")->valuestring);
			
			 this_acts=cJSON_CreateObject();
                		
		         cJSON_AddItemReferenceToObject(this_acts,cJSON_GetObjectItem(comp,"id")->valuestring,temp2);
		         cJSON_AddItemReferenceToArray(this_acts_array,this_acts);
			

			
		 }
	       
		
		}
     
		


    /* Add dependencies to activities */
    for (i=0; i< cJSON_GetArraySize(deps); i++){
	cJSON * dep= cJSON_GetArrayItem(deps,i);
    
	
	
	for (j=0; j< cJSON_GetArraySize(this_acts_array); j++){
		cJSON * obj= cJSON_GetArrayItem(this_acts_array, j);
		
		if (!obj) {printf("Error before: [%s]\n",cJSON_GetErrorPtr());}
		

		//printf("=I am here=\n");
		
		 // out=cJSON_Print(dep);
		// printf("%s\n",out);

		 out=cJSON_Print(obj);
		 //printf("%s\n",out);
		
	 	 char *Cidd=malloc(20);
		 //sprintf(Cid,"%.9s",out);
		 Cidd=strtok(out,":");
		 char* Cid=Cidd+4; 
		 Cid[strlen(Cid)-1] = 0;
		
   		 /*if (strcmp(Cid,cJSON_GetObjectItem(dep,"a1")->valuestring)==0 || strncmp(Cid,cJSON_GetObjectItem(dep,"a2")->valuestring)==0)
			printf("Yahoo\n");*/
		 char *pch = strstr(Cid,cJSON_GetObjectItem(dep,"a1")->valuestring);
		/* out=cJSON_Print(dep);
     		 printf("%s\n",out);*/
		 
                // printf("(Ci)%s\n(a1)%s\n(a2)%s\n",Cid,cJSON_GetObjectItem(dep,"a1")->valuestring, cJSON_GetObjectItem(dep,"a2")->valuestring);
		
		// printf("=I am here=%s %s\n",pch,cJSON_GetObjectItem(dep,"a1")->valuestring);
		 if(pch && (strlen(Cid)==strlen(cJSON_GetObjectItem(dep,"a1")->valuestring))){
			// printf("=I am here=b1 %s\n", Cid);
			
			b1= cJSON_GetObjectItem(obj,cJSON_GetObjectItem(dep,"a1")->valuestring);
			//out=cJSON_Print(b1);
	 	 	//printf("%s\n",out);
			
		}
                
		
		char *pch1 = strstr(Cid, cJSON_GetObjectItem(dep,"a2")->valuestring);
	//	printf("=I am here=%s %s\n",pch1,cJSON_GetObjectItem(dep,"a2")->valuestring);		 
		if(pch1 &&  (strlen(Cid)==strlen(cJSON_GetObjectItem(dep,"a2")->valuestring))){
			// printf("=I am here=b2 %s\n",Cid);
			b2= cJSON_GetObjectItem(obj,cJSON_GetObjectItem(dep,"a2")->valuestring);
			//out=cJSON_Print(b2);
	 	 	//printf("%s\n",out);
			}
	// printf("End of the loop %d, %d\n",j,cJSON_GetArraySize(this_acts_array));	
	}
	
	

 
	
	/* Add to a2 that 'a2 depends on a1' */

	temp=cJSON_CreateObject();
	temp_array=cJSON_CreateArray();
	cJSON_AddStringToObject(temp,"id",cJSON_GetObjectItem(b1,"id")->valuestring);

	

	cJSON_AddNumberToObject(temp,"time",cJSON_GetObjectItem(dep,"time")->valueint);

	
	
	
	if (cJSON_HasObjectItem(b2,"deps"))
		cJSON_AddItemReferenceToArray(cJSON_GetObjectItem(b2,"deps"),temp);
	else{
		cJSON_AddItemToArray(temp_array,temp);		
		cJSON_AddItemReferenceToObject(b2,"deps",temp_array);
	}

	

	temp=cJSON_CreateObject();
	temp_array=cJSON_CreateArray();
	cJSON_AddStringToObject(temp,"id",cJSON_GetObjectItem(b2,"id")->valuestring);
	cJSON_AddNumberToObject(temp,"time",cJSON_GetObjectItem(dep,"time")->valueint);	
	
	

	if (cJSON_HasObjectItem(b1,"triggers"))
		cJSON_AddItemReferenceToArray(cJSON_GetObjectItem(b1,"triggers"),temp);
	else{
		cJSON_AddItemToArray(temp_array,temp);
		cJSON_AddItemReferenceToObject(b1,"triggers",temp_array);
		
	}

	
	
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
