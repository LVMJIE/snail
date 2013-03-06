
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "ictclas50.h"
#include "text_ws.h"


text_ws::text_ws(int init_ws_sys, char *ws_dir)
{
	ws_inited = 0;
	st = 0;

	if (init_ws_sys) {
		ws_inited = 1;
		if (true != ICTCLAS_Init(ws_dir)) {
			st = -1;
		}
	}

	paragraphs.clear();
}

text_ws::~text_ws()
{
	if (ws_inited) ICTCLAS_Exit();
}

int text_ws::parse_file(char *file, unsigned int code_type)
{
	int fd = 0;
	int len = 0;
	char *buf = NULL;
	struct stat st;

	fd = open(file, O_RDONLY);
	if (fd < 0) return -1;

	memset(&st, 0x00, sizeof(struct stat));
	if (0 != stat(file, &st)) {
		return -1;
	}

	buf = (char*)malloc(st.st_size);
	if (!buf) return -1;

	read(fd, buf, st.st_size);
	parse_string(buf, st.st_size, code_type);

	close(fd);

	return 0;
}


int text_ws::parse_string(char *data, int data_len, unsigned int code_type)
{
	int i = 0;
	int res_count = 0;
	LPICTCLAS_RESULT res;
	sentence sent;
	paragraph para;
	char * p;

	para.clear();
	sent.clear();
	this->paragraphs.clear();
	res = ICTCLAS_ParagraphProcessA(data, data_len, res_count, (eCodeType)code_type, true);

	for (i=0; i<res_count; i++) {
		p = res[i].iStartPos+data;
		if (memcmp(p, "。", sizeof("。"))==0 || memcmp(p, "!", sizeof("!"))==0 || memcmp(p, "！", sizeof("！"))==0 || memcmp(p, ".", sizeof("."))==0) {
			// if word is '。' or '!' , then a sentence ends.
			para.push_back(sent);
			sent.clear();
		} else if (memcmp(p, "\n", 1)==0 || (memcmp(p, "\r\n", 2)==0)) {
			// if word is '\n' or '\r\n', then a paragraph ends.
			this->paragraphs.push_back(para);				
			para.clear();
		} else {
			if (res[i].iLength > 0 && *p != ' ') {
				cn_word wd(p, res[i].iLength);
				sent.push_back(wd);
			}
		}
	}

	if (sent.size() > 0) {
		para.push_back(sent);
	}

	if (para.size() > 0) {
		this->paragraphs.push_back(para);
	}

	ICTCLAS_ResultFree(res);
}


int text_ws::show_paragraph()
{
	std::vector<paragraph>::iterator para_ib;	
	std::vector<sentence>::iterator sent_ib;
	std::vector<cn_word>::iterator wd_ib;

	for (para_ib=this->paragraphs.begin(); para_ib != this->paragraphs.end(); para_ib++) {
		fprintf(stdout, "Paragraph:\n");
		for (sent_ib = para_ib->begin(); sent_ib!=para_ib->end(); sent_ib++) {
			fprintf(stdout, "\tSentence:\n\t");
			for (wd_ib = sent_ib->begin(); wd_ib!=sent_ib->end(); wd_ib++) {
				wd_ib->show();
			}
		}
	}

	return 0;
}