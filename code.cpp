#pragma warning(disable:4996)
#include<iostream>
#include <stdlib.h>
#include <string.h>
#include<stdio.h>
#include<iomanip>
#include <fstream>
using namespace std;

#define ERROR  -1
#define OK 0
#define BYTESIZE 256
#define MAXVALUE 999999
#define MaxSize 50
#define MAX 32767
int number[100000000];
int d_count = 0;
int new_huf = 0;
int total_huf = 1;
int total_test = 0;
int fir = 0;
// Huffman树节点
struct HuffNode
{
	unsigned char ch;  //字节符
	int weight;  //字节出现频度
	int parent; //父节点
	int lchild;  //左孩子
	int rchild;  //右孩子
	char bits[256]; // 哈夫曼编码
};
struct HEAD
{
	char type[4];		// 图像类型
	int length;		// 原图像长度
	int weight[BYTESIZE];	// 权值数值
};
//图像结构体
struct FILESTRUCT
{
	int sum; //出现频率
	int fileid;
};
// Huffman树
typedef char** HuffmanCode;

int CreateHuffmanTree(HuffNode *huf_tree, int n);//构造huffman树;        /* 生成Huffman树;  */
void select(HuffNode *huf_tree, int n, int *s1, int *s2);    /* 查找最小和次小序号;  */
int HuffmanCoding(HuffNode *huf_tree, int n);            /* 生成Huffman编码;  */
int extract(HuffNode huf_tree[], char *ifname, char *ofname);
int compress(HuffNode huf_tree[], int n, long flength, char *ifname, char *ofname);					 


//读取图像信息
int readFILE(HuffNode hufftree[], const char* InFile)
{
	int ch;
	FILESTRUCT z[256] = { 0 };
	for (int k = 0; k < 256; k++)//初始化
		z[k].fileid = k;
	FILE *fin = fopen(InFile, "rb");
	// 扫描图像每一个像素，根据Huffman编码表对其进行压缩，压缩结果暂存到缓冲区中
	while ((ch = fgetc(fin)) != EOF)
	{
		number[d_count++] = ch;
		total_test++;
		for (int g = 0; g < 256; g++)
		{
			if (ch == g) //统计字符频度
				z[g].sum++;
		}
	}
	for (int m = 0; m < 256; m++)//冒泡排序
	{
		for (int n = m + 1; n <= 255; n++)
		{
			if (z[n].sum <= z[m].sum)//频度从小到大排序
			{
				int temp1 = z[n].sum;
				z[n].sum = z[m].sum;
				z[m].sum = temp1;

				int temp2 = z[n].fileid;
				z[n].fileid = z[m].fileid;
				z[m].fileid = temp2;
			}
		}
	}
	for (int m = 0; m < 256; m++)//冒泡排序
	{
		for (int n = m + 1; n <= 255; n++)
		{
			if (z[n].sum <= z[m].sum)//序号排序
			{
				int temp2 = z[n].fileid;
				z[n].fileid = z[m].fileid;
				z[m].fileid = temp2;
			}
		}
	}
	int d = 0;//有效字节
	for (int j = 0; j < 256; j++)//去掉频度为0的字符
	{
		if (z[j].sum != 0)
		{
			hufftree[d].ch = z[j].fileid;
			hufftree[d].weight = z[j].sum;
			d++;
		}
	}
	fclose(fin);//关闭图像
	return d;//返回字符个数
}

//构建huffmantree
int CreateHuffmanTree(HuffNode *huf_tree, int n)//构造huffman树
{
	int i;
	int s1, s2;
	for (i = n; i < 2 * n - 1; ++i)
	{
		select(huf_tree, i, &s1, &s2);		// 选择最小的两个结点
		huf_tree[s1].parent = i;            //原点双亲为i
		huf_tree[s2].parent = i;
		huf_tree[i].lchild = s1;               //新结点左子树是最小的s1
		huf_tree[i].rchild = s2;               //新结点右子树是最小s2
		huf_tree[i].weight = huf_tree[s1].weight + huf_tree[s2].weight;////新结点的权值
	}
	return OK;
}

// 选择最小的两个结点
void select(HuffNode *huf_tree, int n, int *s1, int *s2)//在HT[1~I-1】选择parent为零且为最小的两个数，序号分别为s1,s2
{
	// 找最小结点
	unsigned int i;
	unsigned long min = LONG_MAX;
	for (i = 0; i < n; i++)
		if (huf_tree[i].parent == -1 && huf_tree[i].weight < min)
		{
			min = huf_tree[i].weight;
			*s1 = i;//记录下标
		}
	huf_tree[*s1].parent = 1;   // 标记此结点已被选中
								// 找次小结点
	min = LONG_MAX;
	for (i = 0; i < n; i++)
	{
		if (huf_tree[i].parent == -1 && huf_tree[i].weight < min)
		{
			min = huf_tree[i].weight;
			*s2 = i;
		}
	}
}

//从叶子向根求编码
int HuffmanCoding(HuffNode *huf_tree, int n)
{
	int i;
	int cur, next, index;
	char code_tmp[256];		// 暂存编码，最多256个叶子，编码长度不超多255
	code_tmp[255] = '\0';

	for (i = 0; i < n; ++i)
	{
		index = 256 - 1;	// 编码临时空间初始化
		// 从叶子向根求编码
		for (cur = i, next = huf_tree[i].parent; next != -1; next = huf_tree[next].parent)
		{
			if (huf_tree[next].lchild == cur)
				code_tmp[--index] = '0';	// 左‘0’
			else
				code_tmp[--index] = '1';	// 右‘1’

			cur = next;

		}
		strcpy(huf_tree[i].bits, &code_tmp[index]);     // 正向保存编码到树结点相应域 index是第一个
	}
	return OK;
}


//压缩具体实现
int compress(HuffNode huf_tree[], int n, long flength, char *ifname, char *ofname)
{
	FILE * inFile = fopen(ifname, "rb");//打开要压缩图像
	FILE * outFile = fopen(ofname, "wb");//打开压缩后图像

	unsigned char temp = '\0';            //8bit临时的变量

	char buffer[256] = "\0";           //缓存流

	char tou[20];                     //图像后缀名字符数组
	int z = 0;
	int strLen = strlen(ifname);//图像后缀名长度
	for (int g = strLen - 1; g > 0; g--)//获取图像后缀名（从后面获取）
	{
		if (ifname[g] == '.')//获取图像后缀名最后一个“.”
		{
			for (int k = g; k < strLen; k++)
			{
				z++;
				tou[z] = ifname[k];
			}
		}
	}
	tou[0] = z + '0';//获取图像后缀名长度(转成字符)
	fwrite((char *)&tou, sizeof(char), z + 1, outFile);//存图像头部
	fwrite(&flength, sizeof(long), 1, outFile);//存总长度
	fwrite(&n, sizeof(int), 1, outFile);//存字符的种类
	new_huf = new_huf + (z + 1)*sizeof(char) + sizeof(long) + sizeof(int);

	for (int i = 0; i < n; i++) {//存每个编号对应的字符,权重
		fwrite(&huf_tree[i].ch, sizeof(unsigned char), 1, outFile);
		fwrite(&huf_tree[i].weight, sizeof(long), 1, outFile);
		new_huf = new_huf + sizeof(unsigned char) + sizeof(long);
	}
	//cout << "存储的Huffman树信息及文件头大小=" << new_huf << endl;
	fir = new_huf;
	while (fread(&temp, sizeof(unsigned char), 1, inFile))//图像不为空
	{
		for (int i = 0; i < n; i++)//找对应字符
		{
			if (temp == huf_tree[i].ch)
			{
				for (int f = 0; huf_tree[i].bits[f] == '0' || huf_tree[i].bits[f] == '1'; f++)//过滤掉非0非1的编码（数组带来的弊端）
				{
					strncat(buffer, &huf_tree[i].bits[f], 1);//给缓存流赋值
				}
			}
		}
		while (strlen(buffer) >= 8)//缓存流大于等于8个bits进入循环 
		{
			new_huf++;
			temp = 0;
			for (int i = 0; i < 8; i++)//每8个bits循环一次
			{
				temp = temp << 1;//左移1
				if (buffer[i] == '1')//如果是为1，就按位为1
				{
					temp = temp | 0x01;//在不影响其他位的情况下，最后位置1
				}
			}
			fwrite(&temp, sizeof(unsigned char), 1, outFile);//写入图像
			strcpy(buffer, buffer + 8);//将写入图像的bits删除
		}
	}
	int m = strlen(buffer);//将剩余不足为8的bits的个数给l
	if (m) {
		new_huf++;
		temp = 0;
		for (int i = 0; i < m; i++)
		{
			temp = temp << 1;//移动1
			if (buffer[i] == '1')//如果是为1，就按位为1
				temp = temp | 0x01;
		}
		temp <<= 8 - m;// // 将编码字段从尾部移到字节的高位
		fwrite(&temp, sizeof(unsigned char), 1, outFile);//写入最后一个
	}

	fclose(inFile);
	fclose(outFile);
	return 1;
}//compress

//解压缩具体实现
int extract(HuffNode huf_tree[], char *ifname, char *ofname)
{
	int i;
	char huozui;                          //图像后缀长度
	char tou[20];                         //图像后缀字符
	long flength;                         //图像总长度
	int n;                               //字符种类
	int node_num;                        //结点总数
	unsigned long writen_len = 0;		// 控制图像写入长度
	FILE *infile, *outfile;
	unsigned char code_temp;		// 暂存8bits编码
	unsigned int root;		// 保存根节点索引，供匹配编码使用

	infile = fopen(ifname, "rb");		// 以二进制方式打开压缩图像
	// 判断输入图像是否存在
	if (infile == NULL)
		return -1;

	//读取图像后缀名长度
	fread(&huozui, sizeof(char), 1, infile);
	//字符转数字
	int huozui_du = huozui - '0';
	//读取图像后缀字符
	fread(&tou, sizeof(char), huozui_du, infile); //读取图像后缀字符
	fread(&flength, sizeof(long), 1, infile);    //读取图像总长度
	fread(&n, sizeof(int), 1, infile);          //读取字符种类
	node_num = 2 * n - 1;		// 根据字符种类数，计算建立哈夫曼树所需结点数 

	// 初始化后
	for (int a = 0; a < 512; a++)
	{
		huf_tree[a].parent = -1;
		huf_tree[a].ch = NULL;
		huf_tree[a].weight = -1;
		huf_tree[a].lchild = -1;
		huf_tree[a].rchild = -1;
	}

	// 读取压缩图像前端的字符及对应权重，用于重建哈夫曼树
	for (i = 0; i < n; i++)
	{
		fread((char *)&huf_tree[i].ch, sizeof(unsigned char), 1, infile);		// 读入字符
		fread((char *)&huf_tree[i].weight, sizeof(long), 1, infile);	// 读入字符对应权重
	}

	CreateHuffmanTree(huf_tree, n);//构建哈夫曼仿真指针孩子父亲表示法
	HuffmanCoding(huf_tree, n);//生成哈夫曼编码
	strncat(ofname, tou, huozui_du);

	outfile = fopen(ofname, "wb");		// 打开压缩后将生成的图像
	root = node_num - 1;                //根结点的下标
	while (1)
	{
		fread(&code_temp, sizeof(unsigned char), 1, infile);		// 读取一个字符长度的编码

		// 处理读取的一个字符长度的编码（通常为8位）
		for (i = 0; i < 8; i++)
		{
			// 由根向下直至叶节点正向匹配编码对应字符（逆向）
			if (code_temp & 128)//128是1000 0000   按位与就是编码缓存的最高位是否为1
				root = huf_tree[root].rchild;//为1，root=右子树
			else
				root = huf_tree[root].lchild;//为0，root=左子树

			if (root < n)//0到n-1的左右子树为-1
			{
				fwrite(&huf_tree[root].ch, sizeof(unsigned char), 1, outfile);
				writen_len++;//已编译字符加一
				if (writen_len == flength) break;		// 控制图像长度，跳出内层循环
				root = node_num - 1;        // 复位为根索引，匹配下一个字符
			}
			code_temp <<= 1;		// 将编码缓存的下一位移到最高位，提供匹配
		}
		if (writen_len == flength) break;		// 控制图像长度，跳出外层循环
	}

	//关闭图像
	fclose(infile);
	fclose(outfile);
	return 1;
}//extract()


//压缩框架
int compressfile(int *n, int *flag, char * ifname, char * ofname, HuffNode *hufftree)
{
	printf("请输入操作的图像名：");
	fflush(stdin);		// 清空标准输入流，防止干扰gets函数读取图像名
	cin >> ifname;
	printf("请输入生成的图像名：");
	fflush(stdin);
	cin >> ofname;

	//求图像总字节长度
	FILE *file;
	file = fopen(ifname, "r");
	fseek(file, SEEK_SET, SEEK_END);
	long flength = ftell(file);
	fclose(file);
	total_huf = flength;
	*n = readFILE(hufftree, ifname);//读取图像，并按字节频率排序，返回有效字节（频率不为0）
	CreateHuffmanTree(hufftree, *n);//构建哈夫曼仿真指针孩子父亲表示法
	HuffmanCoding(hufftree, *n);//生成哈夫曼编码
	/*printf("%4s  %4s %9s %9s %9s %9s %13s", "序号", "字节符", "频率", "父节点", "左孩子", "右孩子", "霍夫曼编码");
	printf("\n");
	for (int d = 0; d < 2 * (*n) - 1; d++)
	{
		printf("%4d: %4u,%9d,%9d,%9d,%9d", d, hufftree[d].ch, hufftree[d].weight, hufftree[d].parent, hufftree[d].lchild, hufftree[d].rchild); 
		printf("%9s", " ");
		for (int f = 0; hufftree[d].bits[f] == '0' || hufftree[d].bits[f] == '1'; f++)
			printf("%c", hufftree[d].bits[f]);
		printf("\n");
	}*/
	int isok = compress(hufftree, *n, flength, ifname, ofname);	// 压缩，返回值用于判断是否图像名不存在
	if (isok != -1)
	{
		cout << ifname << "正在压缩中……" << endl;
		cout << ofname << "压缩完毕……" << endl;
	}
	return 1;
}

//解压框架
int decompressfile(char * ifname, char * ofname, int *flag, HuffNode *hufftree)
{
	printf("请输入操作的图像名：");
	fflush(stdin);		// 清空标准输入流，防止干扰gets函数读取图像名
	cin >> ifname;
	printf("请输入生成的图像名：");
	fflush(stdin);
	cin >> ofname;

	int isexit = extract(hufftree, ifname, ofname);		// 解压，返回值用于判断是否图像名不存在

	if (isexit != -1)
	{
		cout << ifname << "正在解压中……\n";
		cout << ofname << "解压完毕……\n";
	}
	return 1;
}

//基于动态规划的图像压缩
//存储像素pi所需的位数
int length(int i)
{
	int k = 1;
	i = i / 2;
	while (i > 0)
	{
		k++;
		i = i / 2;
	}
	return k;
}

void Compress(int n, int p[], int s[], int l[], int b[])
{//n个像素，存在数组p[]中，
	int Lmax = 256, header = 11;
	s[0] = 0;
	for (int i = 1; i <= n; i++)
	{
		b[i] = length(p[i]);//计算像素点p需要的存储位数
		int bmax = b[i];
		s[i] = s[i - 1] + bmax;
		l[i] = 1;

		for (int j = 2; j <= i && j <= Lmax; j++)//i=1时，不进入此循环
		{
			if (bmax < b[i - j + 1])
			{
				bmax = b[i - j + 1];
			}

			if (s[i] > s[i - j] + j * bmax)
			{
				s[i] = s[i - j] + j * bmax;
				l[i] = j;
			}
		}
		s[i] += header;
	}
}


void Traceback(int n, int& i, int s[], int l[])
{
	if (n == 0)
		return;
	Traceback(n - l[n], i, s, l);//p1,p2,p3,...,p(n-l[n])像素序列，最后一段有l[n]个像素
	s[i++] = n - l[n];//重新为s[]数组赋值，用来存储分段位置，最终i为共分了多少段
}

void Output(int s[], int l[], int b[], int n)
{

	//cout << "压缩前图像大小为：" << total_huf << "B" << endl;
	//在输出s[n]存储位数后，s[]数组则被重新赋值，用来存储分段的位置
	s[n] = (s[n] % 8) ? s[n] / 8 + 1 : s[n] / 8;
	cout << "图像压缩后的最小空间为：" << s[n] <<"B"<< endl;
	double rate = total_huf * 1.0 / s[n];
	cout.setf(ios::fixed);
	cout << "压缩率为：" << setprecision(4) << rate << "%" << endl;
	int m = 0;

	//Traceback(n, m, s, l);//s[0]:第一段从哪分，s[1]:第二段从哪分...，s[m-1]第m段从哪分
	//s[m] = n;//此时m为总段数，设s[m]=n，分割位置为第n个像素
	//cout << "将原灰度序列分成" << m << "段序列段" << endl;
	//for (int j = 1; j <= m; j++)
	//{
	//	l[j] = l[s[j]];
	//	b[j] = b[s[j]];
	//}
	//for (int j = 1; j <= m; j++)
	//	cout << "段长度：" << l[j] << ",所需存储位数:" << b[j] << endl;
}

void  showmenu()
{
	printf(" =======================================\n");
	cout << "        ***图像压缩算法***" << endl;
	printf(" =======================================\n");
	printf("            ※  1：压缩              \n");
	printf("            ※  2：解压              \n");
	printf("            ※  3：退出              \n");
	printf("         ※ 请输入您的操作：         \n");
}


int main()
{
	//变量段定义
	int n = BYTESIZE;//有效字节
	int opt, flag = 1;
	HuffNode hufftree[512];//最多511个结点

	while (1)
	{
		/*菜单*/
		showmenu();
		/*变量初始化*/
		opt = flag = 0;	// 每次进入循环都要初始化flag为0
		char ifname[256];
		char ofname[256];		// 保存输入输出图像名
		for (int a = 0; a < 512; a++)
		{
			hufftree[a].parent = -1;
			hufftree[a].ch = NULL;
			hufftree[a].weight = -1;
			hufftree[a].lchild = -1;
			hufftree[a].rchild = -1;
		}
		//table逻辑
		scanf("%d", &opt);
		switch (opt) {

		case 1:
		{
			cout << "===========基于Huffman的图像压缩===============" << endl;
			compressfile(&n, &flag, ifname, ofname, hufftree);
			cout << "压缩前图像大小为：" << total_huf << "B" << endl;
			cout << "压缩后图像大小为：" << new_huf << "B" << endl;
			//double rate1 = fir * 1.0 / new_huf;
			//cout.setf(ios::fixed);
			//cout << "存储的Huffman树信息及文件头所占空间与图像总大小比值" << rate1 << endl;
			double rate2 = total_huf * 1.0 / new_huf;
			cout.setf(ios::fixed);
			cout << "压缩率为：" << setprecision(4) << rate2 << "%" << endl;


			cout << "===========基于动态规划的图像压缩===============" << endl;
			int *s, *l, *b;
			int N = total_huf;
			s = new int[N];
			l = new int[N];
			b = new int[N];
			int count_100 = 0, count_200 = 0, count_255 = 0;
			Compress(N - 1, number, s, l, b);
			Output(s, l, b, N - 1);

			for (int i = 0; i < d_count; i++)
			{
				int temp = number[i] / 100;
				switch (temp)
				{
				case(0):
					count_100++;
					break;
				case(1):
					count_200++;
					break;
				case(2):
					count_255++;
					break;
				default:
					count_100++;
					break;
				}
			}
			//cout << "图像全部像素点的个数=" <<d_count << endl;
			//cout << "像素值小于100像素点的个数=" << count_100 << endl;
			//cout << "像素值小于200大于等于100像素点的个数=" << count_200 << endl;
			//cout << "像素值小于255大于等于200像素点的个数" <<count_255 << endl;
			//double rate_100 = count_100 * 1.0 / d_count;
			//double rate_200 = count_200 * 1.0 / d_count;
			//double rate_255 = count_255 * 1.0 / d_count;
			//cout.setf(ios::fixed);
			//cout << "像素值小于100像素点的概率=" << rate_100 << endl;
			//cout.setf(ios::fixed);
			//cout << "像素值小于200大于等于100像素点的概率=" << rate_200 << endl;
			//cout.setf(ios::fixed);
			//cout << "像素值小于255大于等于200像素点的概率=" << rate_255 << endl;
			break;
		}
		case 2:
		{
			decompressfile(ifname, ofname, &flag, hufftree);
			break;
		}
		case 3:
			return 0;
		default:
			;
		}
	}
	return 0;
}
