#define _POSIX_C_SOURCE 200809L
#include "lexer.h"
#include "parser.h"
#include "compiler.h"
#include "vm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

static char*read_file(const char*p){FILE*f=fopen(p,"rb");if(!f){perror(p);return NULL;}if(fseek(f,0,SEEK_END)!=0){fclose(f);return NULL;}long n=ftell(f);if(n<0){fclose(f);return NULL;}rewind(f);char*s=malloc((size_t)n+1);if(!s)exit(2);size_t got=fread(s,1,(size_t)n,f);fclose(f);s[got]=0;return s;}
static void usage(const char*p){printf("greenServeFE - greenServe fast execution VM\n\nUsage:\n  %s file.gsve\n  %s --dump file.gsve\n  %s --trace file.gsve\n  %s --version\n",p,p,p,p);}

static int diagnostic_location(const char *text,int *line,int *column){
    const char *code=strstr(text,"[LEX");
    if(!code)code=strstr(text,"[PAR");
    if(!code)code=strstr(text,"[CMP");
    if(!code)code=strstr(text,"[RUN");
    if(!code)return 0;
    const char *at=strstr(code,"] at ");
    if(!at)return 0;
    at+=5;
    while(*at&&!isdigit((unsigned char)*at))at++;
    if(!*at)return 0;
    char *end=NULL;
    long l=strtol(at,&end,10);
    if(l<=0||!end||*end!=':')return 0;
    end++;
    long c=0;
    if(isdigit((unsigned char)*end))c=strtol(end,NULL,10);
    *line=(int)l;
    *column=(int)c;
    return 1;
}

static void print_source_context(FILE *out,const char *source,int line,int column){
    if(!source||line<=0)return;
    const char *p=source;
    int current=1;
    while(current<line&&*p){if(*p=='\n')current++;p++;}
    if(current!=line)return;
    const char *start=p;
    while(*p&&*p!='\n')p++;
    size_t len=(size_t)(p-start);
    fprintf(out,"  %4d | %.*s\n",line,(int)len,start);
    fprintf(out,"         | ");
    if(column>0){
        size_t target=(size_t)(column-1);
        if(target>len)target=len;
        for(size_t i=0;i<target;i++)fputc(start[i]=='\t'?'\t':' ',out);
        fputc('^',out);
    }else{
        fputc('^',out);
    }
    fputc('\n',out);
}

static void print_diagnostics_with_source(FILE *captured,const char *source){
    rewind(captured);
    char linebuf[8192];
    while(fgets(linebuf,sizeof linebuf,captured)){
        fputs(linebuf,stderr);
        int line=0,column=0;
        if(diagnostic_location(linebuf,&line,&column))print_source_context(stderr,source,line,column);
    }
}

int main(int argc,char**argv){
    if(argc<2||argc>3){usage(argv[0]);return 2;}
    if(!strcmp(argv[1],"--version")){puts("greenServeFE 1.0.2");return 0;}
    int dump=0,trace=0;
    const char*path=argv[1];
    if(!strcmp(argv[1],"--dump")||!strcmp(argv[1],"--trace")){
        if(argc!=3){usage(argv[0]);return 2;}
        dump=!strcmp(argv[1],"--dump");
        trace=!strcmp(argv[1],"--trace");
        path=argv[2];
    }
    const char*ext=strrchr(path,'.');
    if(!ext||strcmp(ext,".gsve")){fprintf(stderr,"greenServeFE: I expected a .gsve source file, but the input path has a different extension.\n");return 2;}

    char*src=read_file(path);
    if(!src)return 1;

    FILE*captured=tmpfile();
    if(!captured){free(src);perror("greenServeFE: tmpfile");return 1;}
    int saved_stderr=dup(STDERR_FILENO);
    if(saved_stderr<0){fclose(captured);free(src);perror("greenServeFE: dup");return 1;}
    fflush(stderr);
    if(dup2(fileno(captured),STDERR_FILENO)<0){close(saved_stderr);fclose(captured);free(src);perror("greenServeFE: dup2");return 1;}

    int rc=0;
    TokenList toks=lex_source(src);
    for(size_t i=0;i<toks.count;i++)if(toks.items[i].type==TOK_ERROR){rc=1;break;}

    Program p={0};
    GSCode*code=NULL;
    GSVM*vm=NULL;
    if(!rc){
        p=parse_program(&toks);
        if(p.errors)rc=1;
    }
    free_tokens(&toks);

    if(!rc){
        code=compile_program(&p);
        if(!code)rc=1;
    }
    free_program(&p);

    if(!rc){
        if(dump)code_disassemble(code,"");
        vm=vm_new();
        vm_set_trace(vm,trace);
        rc=vm_run(vm,code);
        vm_free(vm);
        code_release(code);
    }

    fflush(stderr);
    fflush(captured);
    if(dup2(saved_stderr,STDERR_FILENO)<0){close(saved_stderr);fclose(captured);free(src);return 1;}
    close(saved_stderr);

    print_diagnostics_with_source(captured,src);
    fclose(captured);
    free(src);
    return rc;
}
