#ifndef REVIEW_H
#define REVIEW_H

void review_tcp(const char *payload,db_ip_t *db_ip);

void review_udp(const char *payload,db_ip_t *db_ip);

//void review_process(const char *payload,db_ip_t *db_ip);

//telnet 协议帧
//下一个状态为数据 下一个状态为命令字 下一个状态为协商选项

enum _telnet_cmd
{
	NUL=0,
	BEL=7,
	BS=8,
	HT=9,
	LF=10,
	VT=11,
	FF=12,
	CR=13,
	SE=240,
	NOP=241,
	DM=242,
	BRK=243,
	IP=244,
	AO=245,
	AYT=246,
	EC=247,
	EL=248,
	GA=249,
	SB=250,
	WILL=251,	//选项协商
	WONT=252,	//选项协商
	DO=253,		//选项协商
	DONT=254,	//选项协商
	IAC=255		//字符0XFF
};

enum _telnet_option
{
	TOPT_BIN = 0,   // Binary Transmission
	TOPT_ECHO = 1,  // Echo
	TOPT_RECN = 2,  // Reconnection
	TOPT_SUPP = 3,  // Suppress Go Ahead
	TOPT_APRX = 4,  // Approx Message Size Negotiation
	TOPT_STAT = 5,  // Status
	TOPT_TIM = 6,   // Timing Mark
	TOPT_REM = 7,   // Remote Controlled Trans and Echo
	TOPT_OLW = 8,   // Output Line Width
	TOPT_OPS = 9,   // Output Page Size
	TOPT_OCRD = 10, // Output Carriage-Return Disposition
	TOPT_OHT = 11,  // Output Horizontal Tabstops
	TOPT_OHTD = 12, // Output Horizontal Tab Disposition
	TOPT_OFD = 13,  // Output Formfeed Disposition
	TOPT_OVT = 14,  // Output Vertical Tabstops
	TOPT_OVTD = 15, // Output Vertical Tab Disposition
	TOPT_OLD = 16,  // Output Linefeed Disposition
	TOPT_EXT = 17,  // Extended ASCII
	TOPT_LOGO = 18, // Logout
	TOPT_BYTE = 19, // Byte Macro
	TOPT_DATA = 20, // Data Entry Terminal
	TOPT_SUP = 21,  // SUPDUP
	TOPT_SUPO = 22, // SUPDUP Output
	TOPT_SNDL = 23, // Send Location
	TOPT_TERM = 24, // Terminal Type
	TOPT_EOR = 25,  // End of Record
	TOPT_TACACS = 26, // TACACS User Identification
	TOPT_OM = 27,   // Output Marking
	TOPT_TLN = 28,  // Terminal Location Number
	TOPT_3270 = 29, // Telnet 3270 Regime
	TOPT_X3 = 30,  // X.3 PAD
	TOPT_NAWS = 31, // Negotiate About Window Size
	TOPT_TS = 32,   // Terminal Speed
	TOPT_RFC = 33,  // Remote Flow Control
	TOPT_LINE = 34, // Linemode
	TOPT_XDL = 35,  // X Display Location
	TOPT_ENVIR = 36,// Telnet Environment Option
	TOPT_AUTH = 37, // Telnet Authentication Option
	TOPT_NENVIR = 39,// Telnet Environment Option
	TOPT_EXTOP = 255, // Extended-Options-List
	TOPT_ERROR = 256  // Magic number
};

void process_telnet(const char *payload,db_ip_t *db_ip);

void process_ftp_path1(const char *payload,db_ip_t *db_ip);

#endif
