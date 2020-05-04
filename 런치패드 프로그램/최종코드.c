#include <stdio.h>
#include <Windows.h>
#include <conio.h>
#include <process.h>
#include <stdlib.h>
#include <time.h>
#include <MMSystem.h>
#include "wave.h"

#pragma comment(lib,"winmm.lib")

/* ���� ���� ��ũ�� */
#define bps 32
#define channel 2
#define duration 180
#define sampleRate 44100

/* ���� ���� ��ũ�� */
#define HD GetStdHandle(STD_OUTPUT_HANDLE)
#define GREEN SetConsoleTextAttribute((HD), 10);
#define VOILET SetConsoleTextAttribute((HD), 13);
#define LIGHT_BLUE SetConsoleTextAttribute((HD), FOREGROUND_BLUE | FOREGROUND_INTENSITY);
#define LIGHT_AQUA SetConsoleTextAttribute((HD), FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
#define LIGHT_RED SetConsoleTextAttribute((HD), FOREGROUND_RED | FOREGROUND_INTENSITY);
#define LIGHT_PURPLE SetConsoleTextAttribute((HD), FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
#define LIGHT_YELLOW SetConsoleTextAttribute((HD), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
#define LIGHT_WHITE SetConsoleTextAttribute((HD), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);

/* ������Ʈ���� ���� ��ũ�� */
#define MUSIC_PLAY "play "
#define MUSIC_STOP "stop "
#define MUSIC_PAUSE "pause "
#define MUSIC_RESUME "resume "
#define MUSIC_CLOSE "close "
#define LINK "c:\\Project\\"
#define FILE_EXTENSION ".wav"
#define RECORD_LINK "c:\\Project\\record\\"

/* ��Ÿ ��ũ�� */
#define DELAY 8
#define BUTTON_X 39
#define BUTTON_Y 13

CRITICAL_SECTION m_csGDILock;		// ������ ����ȭ ����

/* �������̽� ���� �Լ� */
void INTERFACE01();
void INTERFACE02();

/* �޴� ���� �Լ� */
void MENU();
void SELECT_MU(int sel_mu_link);

/* ��ġ�е� ȭ�� �� Ű ���� ���� �Լ� */
void PLAY_LP(int pl_link);
void SELECT_LP_1(int sel_lp_1_link_1, int sel_lp_1_link_2);

/* ���� ���� �ҷ����� ���� �Լ� */
void RECORD_LOAD();
void RECORD_UNLOAD();
void record_search();
WIN32_FIND_DATA FileData[100];

/* ���丮�� ���� �Լ� */
void TUTO_1();
void TUTO_2();

/* ���� ���� �Լ� */
void START_RECORD(int record__link);
void IMMEDIATELY_PLAY(char *record_file_name);
void SELECT_RECORD_1(int sel_rec__value_1);
void SELECT_RECORD_2();
void SELECT_RECORD_3(int sel_rec__value_3);

/* ������Ʈ���� ���� �Լ� */
void PROJECT_1(int pr_1_link);
void SELECT_LP_2(int sel_lp_2_link);
void Project_select(int pro_sel_num);

/* ��Ÿ �Լ� */
void REMOVECURSOR();
void GOTOXY(int x, int y);
void BUTTON(int bu_x, int bu_y);

/* ���� ���� ������ �����ϴ� ����ü */
struct WAVEHDR
{
	char *lpData; 
    unsigned long dwBufferLength; 
    unsigned long dwBytesRecorded; 
    unsigned long *dwUser; 
    unsigned long dwFlags; 
    unsigned long dwLoops; 
    struct WAVEHDR *lpNext; 
    unsigned long *reserved;
}WaveInHdr;

unsigned short ttt = 0;					// ���� �����ؼ� ����ϴ� ����
short waveIn[sampleRate * duration * channel * (int)(bps / 8)];		// ���� �����ؼ� ����ϴ� ����
int count_record = 0, record_value = 0;			// ���� �����ؼ� ����ϴ� ����

/* �����忡�� ��� �� ���� */
int key_state[30] = { 0 }; 
int last_state[30] = { 0 };
int count[30] = { 0 };

/* ������Ʈ���� ���� ���� */
char Sel_project_start[30][100]={0};
char Sel_project_stop[30][100]={0};

/* ������ �� Ű���� ���� (Ű �Է½� ���� ��� �Է� ��ҽ� ���� ����) */
void __stdcall T0(void *arg)			// 'Q'
{ 
	while (1)
	{
		key_state[0] = GetAsyncKeyState('Q');		// Ű �Է� ���¸� �����ϴ� �Լ�
		if (key_state[0] & 0x8000)					// Ű�� �Է� �Ǿ��� ���
		{
			EnterCriticalSection(&m_csGDILock);		// ������ ����ȭ ����
			LIGHT_BLUE BUTTON(BUTTON_X,BUTTON_Y);	// �� ���
			LeaveCriticalSection(&m_csGDILock);		// ������ ����ȭ ����
			Sleep(DELAY);
			mciSendString(Sel_project_start[0], NULL, 0, NULL);		// Ű�� �´� ���� ���
																	// Sel_project_start -> ���ڿ� ���� (�Ʒ� ����)
			last_state[0] = 1;
		}
		else
		{
			if (last_state[0] == 1)									// ���� Ű �Է��� ���� �ʰ� �ִ� ���� �̸�, ������ Ű�� ������ �־��� ���
			{
				EnterCriticalSection(&m_csGDILock);
				LIGHT_WHITE BUTTON(BUTTON_X,BUTTON_Y);
				LeaveCriticalSection(&m_csGDILock);
				mciSendString(Sel_project_stop[0], NULL, 0, NULL);	// ���� ����
				last_state[0] = 0;
				count[0] = 0;
				return;												// ������ ����
			}
		}
		
	}
}

void __stdcall T1(void *arg)			// 'W'
{
	while (1) 
	{
		key_state[1] = GetAsyncKeyState('W');
		if (key_state[1] & 0x8000) 
		{
			EnterCriticalSection(&m_csGDILock);
			LIGHT_AQUA BUTTON(BUTTON_X+10,BUTTON_Y);
			LeaveCriticalSection(&m_csGDILock);
			Sleep(DELAY);
			mciSendString(Sel_project_start[1], NULL, 0, NULL);
			last_state[1] = 1;
		}
		else
		{
			if (last_state[1] == 1) 
			{
				EnterCriticalSection(&m_csGDILock);
				LIGHT_WHITE BUTTON(BUTTON_X+10,BUTTON_Y);
				LeaveCriticalSection(&m_csGDILock);
				mciSendString(Sel_project_stop[1], NULL, 0, NULL);
				last_state[1] = 0;
				count[1] = 0;
				return;
			}
		}
	}
}

void __stdcall T2(void *arg) 			// 'E'
{
	while (1) 
	{
		key_state[2] = GetAsyncKeyState('E');
		if (key_state[2] & 0x8000) 
		{
			EnterCriticalSection(&m_csGDILock);
			LIGHT_PURPLE BUTTON(BUTTON_X+20,BUTTON_Y);
			LeaveCriticalSection(&m_csGDILock);
			Sleep(DELAY);
			mciSendString(Sel_project_start[2], NULL, 0, NULL);
			last_state[2] = 1;
		}
		else {
			if (last_state[2] == 1) 
			{
				EnterCriticalSection(&m_csGDILock);
				LIGHT_WHITE BUTTON(BUTTON_X+20,BUTTON_Y);
				LeaveCriticalSection(&m_csGDILock);
				mciSendString(Sel_project_stop[2], NULL, 0, NULL);
				last_state[2] = 0;
				count[2] = 0;
				return;
			}
		}
	}
}

void __stdcall T3(void *arg) 			// 'R'
{
	while (1) 
	{
		key_state[3] = GetAsyncKeyState('R');
		if (key_state[3] & 0x8000) 
		{
			EnterCriticalSection(&m_csGDILock);
			LIGHT_YELLOW BUTTON(BUTTON_X+30,BUTTON_Y);
			LeaveCriticalSection(&m_csGDILock);
			Sleep(DELAY);
			mciSendString(Sel_project_start[3], NULL, 0, NULL);
			last_state[3] = 1;
		}
		else {
			if (last_state[3] == 1) 
			{
				EnterCriticalSection(&m_csGDILock);
				LIGHT_WHITE BUTTON(BUTTON_X+30,BUTTON_Y);
				LeaveCriticalSection(&m_csGDILock);
				mciSendString(Sel_project_stop[3], NULL, 0, NULL);
				last_state[3] = 0;
				count[3] = 0;
				return;
			}
		}
	}
}

void __stdcall T4(void *arg)			// 'T'
{ 
	while (1)
	{
		key_state[4] = GetAsyncKeyState('T');
		if (key_state[4] & 0x8000) 
		{
			EnterCriticalSection(&m_csGDILock);
			LIGHT_RED BUTTON(BUTTON_X+40,BUTTON_Y);
			LeaveCriticalSection(&m_csGDILock);
			Sleep(DELAY);
			mciSendString(Sel_project_start[4], NULL, 0, NULL);
			last_state[4] = 1;
		}
		else
		{
			if (last_state[4] == 1) 
			{
				EnterCriticalSection(&m_csGDILock);
				LIGHT_WHITE BUTTON(BUTTON_X+40,BUTTON_Y);
				LeaveCriticalSection(&m_csGDILock);
				mciSendString(Sel_project_stop[4], NULL, 0, NULL);
				last_state[4] = 0;
				count[4] = 0;
				return;
			}
		}
	}
}

void __stdcall T5(void *arg)			// 'Y'
{ 
	while (1)
	{
		key_state[5] = GetAsyncKeyState('Y');
		if (key_state[5] & 0x8000) 
		{
			EnterCriticalSection(&m_csGDILock);
			LIGHT_BLUE BUTTON(BUTTON_X+50,BUTTON_Y);
			LeaveCriticalSection(&m_csGDILock);
			Sleep(DELAY);
			mciSendString(Sel_project_start[5], NULL, 0, NULL);
			last_state[5] = 1;
		}
		else
		{
			if (last_state[5] == 1) 
			{
				EnterCriticalSection(&m_csGDILock);
				LIGHT_WHITE BUTTON(BUTTON_X+50,BUTTON_Y);
				LeaveCriticalSection(&m_csGDILock);
				mciSendString(Sel_project_stop[5], NULL, 0, NULL);
				last_state[5] = 0;
				count[5] = 0;
				return;
			}
		}
		
	}
}

void __stdcall T6(void *arg)			// 'U'
{ 
	while (1)
	{
		key_state[6] = GetAsyncKeyState('U');
		if (key_state[6] & 0x8000) 
		{
			EnterCriticalSection(&m_csGDILock);
			LIGHT_AQUA BUTTON(BUTTON_X,BUTTON_Y+5);
			LeaveCriticalSection(&m_csGDILock);
			Sleep(DELAY);
			mciSendString(Sel_project_start[6], NULL, 0, NULL);
			last_state[6] = 1;
		}
		else
		{
			if (last_state[6] == 1) 
			{
				EnterCriticalSection(&m_csGDILock);
				LIGHT_WHITE BUTTON(BUTTON_X,BUTTON_Y+5);
				LeaveCriticalSection(&m_csGDILock);
				mciSendString(Sel_project_stop[6], NULL, 0, NULL);
				last_state[6] = 0;
				count[6] = 0;
				return;
			}
		}
	}
}

void __stdcall T7(void *arg)			// 'I'
{ 
	while (1)
	{
		key_state[7] = GetAsyncKeyState('I');
		if (key_state[7] & 0x8000) 
		{
			EnterCriticalSection(&m_csGDILock);
			LIGHT_AQUA BUTTON(BUTTON_X+10,BUTTON_Y+5);
			LeaveCriticalSection(&m_csGDILock);
			Sleep(DELAY);
			mciSendString(Sel_project_start[7], NULL, 0, NULL);
			last_state[7] = 1;
		}
		else
		{
			if (last_state[7] == 1) 
			{
				EnterCriticalSection(&m_csGDILock);
				LIGHT_WHITE BUTTON(BUTTON_X+10,BUTTON_Y+5);
				LeaveCriticalSection(&m_csGDILock);
				mciSendString(Sel_project_stop[7], NULL, 0, NULL);
				last_state[7] = 0;
				count[7] = 0;
				return;
			}
		}
	}
}

void __stdcall T8(void *arg)			// 'O'
{ 
	while (1)
	{
		key_state[8] = GetAsyncKeyState('O');
		if (key_state[8] & 0x8000) 
		{
			EnterCriticalSection(&m_csGDILock);
			LIGHT_AQUA BUTTON(BUTTON_X+20,BUTTON_Y+5);
			LeaveCriticalSection(&m_csGDILock);
			Sleep(DELAY);
			mciSendString(Sel_project_start[8], NULL, 0, NULL);
			last_state[8] = 1;
		}
		else
		{
			if (last_state[8] == 1) 
			{
				EnterCriticalSection(&m_csGDILock);
				LIGHT_WHITE BUTTON(BUTTON_X+20,BUTTON_Y+5);
				LeaveCriticalSection(&m_csGDILock);
				mciSendString(Sel_project_stop[8], NULL, 0, NULL);
				last_state[8] = 0;
				count[8] = 0;
				return;
			}
		}
	}
}

void __stdcall T9(void *arg)			// 'P'
{ 
	while (1)
	{
		key_state[9] = GetAsyncKeyState('P');
		if (key_state[9] & 0x8000) 
		{
			EnterCriticalSection(&m_csGDILock);
			LIGHT_AQUA BUTTON(BUTTON_X+30,BUTTON_Y+5);
			LeaveCriticalSection(&m_csGDILock);
			Sleep(DELAY);
			mciSendString(Sel_project_start[9], NULL, 0, NULL);
			last_state[9] = 1;
		}
		else
		{
			if (last_state[9] == 1) 
			{
				EnterCriticalSection(&m_csGDILock);
				LIGHT_WHITE BUTTON(BUTTON_X+30,BUTTON_Y+5);
				LeaveCriticalSection(&m_csGDILock);
				mciSendString(Sel_project_stop[9], NULL, 0, NULL);
				last_state[9] = 0;
				count[9] = 0;
				return;
			}
		}
	}
}

void __stdcall T10(void *arg)			// 'A'
{ 
	while (1)
	{
		key_state[10] = GetAsyncKeyState('A');
		if (key_state[10] & 0x8000) 
		{
			EnterCriticalSection(&m_csGDILock);
			LIGHT_AQUA BUTTON(BUTTON_X+40,BUTTON_Y+5);
			LeaveCriticalSection(&m_csGDILock);
			Sleep(DELAY);
			mciSendString(Sel_project_start[10], NULL, 0, NULL);
			last_state[10] = 1;
		}
		else
		{
			if (last_state[10] == 1) 
			{
				EnterCriticalSection(&m_csGDILock);
				LIGHT_WHITE BUTTON(BUTTON_X+40,BUTTON_Y+5);
				LeaveCriticalSection(&m_csGDILock);
				mciSendString(Sel_project_stop[10], NULL, 0, NULL);
				last_state[10] = 0;
				count[10] = 0;
				return;
			}
		}
	}
}

void __stdcall T11(void *arg)			// 'S'
{ 
	while (1)
	{
		key_state[11] = GetAsyncKeyState('S');
		if (key_state[11] & 0x8000) 
		{
			EnterCriticalSection(&m_csGDILock);
			LIGHT_AQUA BUTTON(BUTTON_X+50,BUTTON_Y+5);
			LeaveCriticalSection(&m_csGDILock);
			Sleep(DELAY);
			mciSendString(Sel_project_start[11], NULL, 0, NULL);
			last_state[11] = 1;
		}
		else
		{
			if (last_state[11] == 1) 
			{
				EnterCriticalSection(&m_csGDILock);
				LIGHT_WHITE BUTTON(BUTTON_X+50,BUTTON_Y+5);
				LeaveCriticalSection(&m_csGDILock);
				mciSendString(Sel_project_stop[11], NULL, 0, NULL);
				last_state[11] = 0;
				count[11] = 0;
				return;
			}
		}
	}
}

void __stdcall T12(void *arg)			// 'D'
{ 
	while (1)
	{
		key_state[12] = GetAsyncKeyState('D');
		if (key_state[12] & 0x8000) 
		{
			EnterCriticalSection(&m_csGDILock);
			LIGHT_AQUA BUTTON(BUTTON_X,BUTTON_Y+10);
			LeaveCriticalSection(&m_csGDILock);
			Sleep(DELAY);
			mciSendString(Sel_project_start[12], NULL, 0, NULL);
			last_state[12] = 1;
		}
		else
		{
			if (last_state[12] == 1) 
			{
				EnterCriticalSection(&m_csGDILock);
				LIGHT_WHITE BUTTON(BUTTON_X,BUTTON_Y+10);
				LeaveCriticalSection(&m_csGDILock);
				mciSendString(Sel_project_stop[12], NULL, 0, NULL);
				last_state[12] = 0;
				count[12] = 0;
				return;
			}
		}
	}
}

void __stdcall T13(void *arg)			// 'F'
{ 
	while (1)
	{
		key_state[13] = GetAsyncKeyState('F');
		if (key_state[13] & 0x8000) 
		{
			EnterCriticalSection(&m_csGDILock);
			LIGHT_AQUA BUTTON(BUTTON_X+10,BUTTON_Y+10);
			LeaveCriticalSection(&m_csGDILock);
			Sleep(DELAY);
			mciSendString(Sel_project_start[13], NULL, 0, NULL);
			last_state[13] = 1;
		}
		else
		{
			if (last_state[13] == 1) 
			{
				EnterCriticalSection(&m_csGDILock);
				LIGHT_WHITE BUTTON(BUTTON_X+10,BUTTON_Y+10);
				LeaveCriticalSection(&m_csGDILock);
				mciSendString(Sel_project_stop[13], NULL, 0, NULL);
				last_state[13] = 0;
				count[13] = 0;
				return;
			}
		}
	}
}

void __stdcall T14(void *arg)			// 'G'
{ 
	while (1)
	{
		key_state[14] = GetAsyncKeyState('G');
		if (key_state[14] & 0x8000) 
		{
			EnterCriticalSection(&m_csGDILock);
			LIGHT_AQUA BUTTON(BUTTON_X+20,BUTTON_Y+10);
			LeaveCriticalSection(&m_csGDILock);
			Sleep(DELAY);
			mciSendString(Sel_project_start[14], NULL, 0, NULL);
			last_state[14] = 1;
		}
		else
		{
			if (last_state[14] == 1) 
			{
				EnterCriticalSection(&m_csGDILock);
				LIGHT_WHITE BUTTON(BUTTON_X+20,BUTTON_Y+10);
				LeaveCriticalSection(&m_csGDILock);
				mciSendString(Sel_project_stop[14], NULL, 0, NULL);
				last_state[14] = 0;
				count[14] = 0;
				return;
			}
		}
	}
}

void __stdcall T15(void *arg)			// 'H'
{ 
	while (1)
	{
		key_state[15] = GetAsyncKeyState('H');
		if (key_state[15] & 0x8000) 
		{
			EnterCriticalSection(&m_csGDILock);
			LIGHT_AQUA BUTTON(BUTTON_X+30,BUTTON_Y+10);
			LeaveCriticalSection(&m_csGDILock);
			Sleep(DELAY);
			mciSendString(Sel_project_start[15], NULL, 0, NULL);
			last_state[15] = 1;
		}
		else
		{
			if (last_state[15] == 1) 
			{
				EnterCriticalSection(&m_csGDILock);
				LIGHT_WHITE BUTTON(BUTTON_X+30,BUTTON_Y+10);
				LeaveCriticalSection(&m_csGDILock);
				mciSendString(Sel_project_stop[15], NULL, 0, NULL);
				last_state[15] = 0;
				count[15] = 0;
				return;
			}
		}
	}
}

void __stdcall T16(void *arg)			// 'J'
{ 
	while (1)
	{
		key_state[16] = GetAsyncKeyState('J');
		if (key_state[16] & 0x8000) 
		{
			EnterCriticalSection(&m_csGDILock);
			LIGHT_AQUA BUTTON(BUTTON_X+40,BUTTON_Y+10);
			LeaveCriticalSection(&m_csGDILock);
			Sleep(DELAY);
			mciSendString(Sel_project_start[16], NULL, 0, NULL);
			last_state[16] = 1;
		}
		else
		{
			if (last_state[16] == 1) 
			{
				EnterCriticalSection(&m_csGDILock);
				LIGHT_WHITE BUTTON(BUTTON_X+40,BUTTON_Y+10);
				LeaveCriticalSection(&m_csGDILock);
				mciSendString(Sel_project_stop[16], NULL, 0, NULL);
				last_state[16] = 0;
				count[16] = 0;
				return;
			}
		}
	}
}

void __stdcall T17(void *arg)			// 'K'
{ 
	while (1)
	{
		key_state[17] = GetAsyncKeyState('K');
		if (key_state[17] & 0x8000) 
		{
			EnterCriticalSection(&m_csGDILock);
			LIGHT_AQUA BUTTON(BUTTON_X+50,BUTTON_Y+10);
			LeaveCriticalSection(&m_csGDILock);
			Sleep(DELAY);
			mciSendString(Sel_project_start[17], NULL, 0, NULL);
			last_state[17] = 1;
		}
		else
		{
			if (last_state[17] == 1) 
			{
				EnterCriticalSection(&m_csGDILock);
				LIGHT_WHITE BUTTON(BUTTON_X+50,BUTTON_Y+10);
				LeaveCriticalSection(&m_csGDILock);
				mciSendString(Sel_project_stop[17], NULL, 0, NULL);
				last_state[17] = 0;
				count[17] = 0;
				return;
			}
		}
	}
}

void __stdcall T18(void *arg)			// 'L'
{ 
	while (1)
	{
		key_state[18] = GetAsyncKeyState('L');
		if (key_state[18] & 0x8000) 
		{
			EnterCriticalSection(&m_csGDILock);
			LIGHT_AQUA BUTTON(BUTTON_X,BUTTON_Y+15);
			LeaveCriticalSection(&m_csGDILock);
			Sleep(DELAY);
			mciSendString(Sel_project_start[18], NULL, 0, NULL);
			last_state[18] = 1;
		}
		else
		{
			if (last_state[18] == 1) 
			{
				EnterCriticalSection(&m_csGDILock);
				LIGHT_WHITE BUTTON(BUTTON_X,BUTTON_Y+15);
				LeaveCriticalSection(&m_csGDILock);
				mciSendString(Sel_project_stop[18], NULL, 0, NULL);
				last_state[18] = 0;
				count[18] = 0;
				return;
			}
		}
	}
}

void __stdcall T19(void *arg)			// 'Z'
{ 
	while (1)
	{
		key_state[19] = GetAsyncKeyState('Z');
		if (key_state[19] & 0x8000) 
		{
			EnterCriticalSection(&m_csGDILock);
			LIGHT_AQUA BUTTON(BUTTON_X+10,BUTTON_Y+15);
			LeaveCriticalSection(&m_csGDILock);
			Sleep(DELAY);
			mciSendString(Sel_project_start[19], NULL, 0, NULL);
			last_state[19] = 1;
		}
		else
		{
			if (last_state[19] == 1) 
			{
				EnterCriticalSection(&m_csGDILock);
				LIGHT_WHITE BUTTON(BUTTON_X+10,BUTTON_Y+15);
				LeaveCriticalSection(&m_csGDILock);
				mciSendString(Sel_project_stop[19], NULL, 0, NULL);
				last_state[19] = 0;
				count[19] = 0;
				return;
			}
		}
	}
}

void __stdcall T20(void *arg)			// 'X'
{ 
	while (1)
	{
		key_state[20] = GetAsyncKeyState('X');
		if (key_state[20] & 0x8000) 
		{
			EnterCriticalSection(&m_csGDILock);
			LIGHT_AQUA BUTTON(BUTTON_X+20,BUTTON_Y+15);
			LeaveCriticalSection(&m_csGDILock);
			Sleep(DELAY);
			mciSendString(Sel_project_start[20], NULL, 0, NULL);
			last_state[20] = 1;
		}
		else
		{
			if (last_state[20] == 1) 
			{
				EnterCriticalSection(&m_csGDILock);
				LIGHT_WHITE BUTTON(BUTTON_X+20,BUTTON_Y+15);
				LeaveCriticalSection(&m_csGDILock);
				mciSendString(Sel_project_stop[20], NULL, 0, NULL);
				last_state[20] = 0;
				count[20] = 0;
				return;
			}
		}
	}
}

void __stdcall T21(void *arg)			// 'C'
{ 
	while (1)
	{
		key_state[21] = GetAsyncKeyState('C');
		if (key_state[21] & 0x8000) 
		{
			EnterCriticalSection(&m_csGDILock);
			LIGHT_AQUA BUTTON(BUTTON_X+30,BUTTON_Y+15);
			LeaveCriticalSection(&m_csGDILock);
			Sleep(DELAY);
			mciSendString(Sel_project_start[21], NULL, 0, NULL);
			last_state[21] = 1;
		}
		else
		{
			if (last_state[21] == 1) 
			{
				EnterCriticalSection(&m_csGDILock);
				LIGHT_WHITE BUTTON(BUTTON_X+30,BUTTON_Y+15);
				LeaveCriticalSection(&m_csGDILock);
				mciSendString(Sel_project_stop[21], NULL, 0, NULL);
				last_state[21] = 0;
				count[21] = 0;
				return;
			}
		}
	}
}

void __stdcall T22(void *arg)			// 'V'
{ 
	while (1)
	{
		key_state[22] = GetAsyncKeyState('V');
		if (key_state[22] & 0x8000) 
		{
			EnterCriticalSection(&m_csGDILock);
			LIGHT_AQUA BUTTON(BUTTON_X+40,BUTTON_Y+15);
			LeaveCriticalSection(&m_csGDILock);
			Sleep(DELAY);
			mciSendString(Sel_project_start[22], NULL, 0, NULL);
			last_state[22] = 1;
		}
		else
		{
			if (last_state[22] == 1) 
			{
				EnterCriticalSection(&m_csGDILock);
				LIGHT_WHITE BUTTON(BUTTON_X+40,BUTTON_Y+15);
				LeaveCriticalSection(&m_csGDILock);
				mciSendString(Sel_project_stop[22], NULL, 0, NULL);
				last_state[22] = 0;
				count[22] = 0;
				return;
			}
		}
	}
}

void __stdcall T23(void *arg)			// 'B'
{ 
	while (1)
	{
		key_state[23] = GetAsyncKeyState('B');
		if (key_state[23] & 0x8000) 
		{
			EnterCriticalSection(&m_csGDILock);
			LIGHT_AQUA BUTTON(BUTTON_X+50,BUTTON_Y+15);
			LeaveCriticalSection(&m_csGDILock);
			Sleep(DELAY);
			mciSendString(Sel_project_start[23], NULL, 0, NULL);
			last_state[23] = 1;
		}
		else
		{
			if (last_state[23] == 1) 
			{
				EnterCriticalSection(&m_csGDILock);
				LIGHT_WHITE BUTTON(BUTTON_X+50,BUTTON_Y+15);
				LeaveCriticalSection(&m_csGDILock);
				mciSendString(Sel_project_stop[23], NULL, 0, NULL);
				last_state[23] = 0;
				count[23] = 0;
				return;
			}
		}
	}
}

void __stdcall T24(void *arg)			// 'N'
{ 
	while (1)
	{
		key_state[24] = GetAsyncKeyState('N');
		if (key_state[24] & 0x8000) 
		{
			EnterCriticalSection(&m_csGDILock);
			LIGHT_AQUA BUTTON(BUTTON_X,BUTTON_Y+20);
			LeaveCriticalSection(&m_csGDILock);
			Sleep(DELAY);
			mciSendString(Sel_project_start[24], NULL, 0, NULL);
			last_state[24] = 1;
		}
		else
		{
			if (last_state[24] == 1) 
			{
				EnterCriticalSection(&m_csGDILock);
				LIGHT_WHITE BUTTON(BUTTON_X,BUTTON_Y+20);
				LeaveCriticalSection(&m_csGDILock);
				mciSendString(Sel_project_stop[24], NULL, 0, NULL);
				last_state[24] = 0;
				count[24] = 0;
				return;
			}
		}
	}
}

void __stdcall T25(void *arg)			// 'M'
{ 
	while (1)
	{
		key_state[25] = GetAsyncKeyState('M');
		if (key_state[25] & 0x8000) 
		{
			EnterCriticalSection(&m_csGDILock);
			LIGHT_AQUA BUTTON(BUTTON_X+10,BUTTON_Y+20);
			LeaveCriticalSection(&m_csGDILock);
			Sleep(DELAY);
			mciSendString(Sel_project_start[25], NULL, 0, NULL);
			last_state[25] = 1;
		}
		else
		{
			if (last_state[25] == 1) 
			{
				EnterCriticalSection(&m_csGDILock);
				LIGHT_WHITE BUTTON(BUTTON_X+10,BUTTON_Y+20);
				LeaveCriticalSection(&m_csGDILock);
				mciSendString(Sel_project_stop[25], NULL, 0, NULL);
				last_state[25] = 0;
				count[25] = 0;
				return;
			}
		}
	}
}

void __stdcall T26(void *arg)			// 'NUMPAD0'
{ 
	while (1)
	{
		key_state[26] = GetAsyncKeyState(VK_NUMPAD0);
		if (key_state[26] & 0x8000) 
		{
			EnterCriticalSection(&m_csGDILock);
			LIGHT_AQUA BUTTON(BUTTON_X+20,BUTTON_Y+20);
			LeaveCriticalSection(&m_csGDILock);
			Sleep(DELAY);
			mciSendString(Sel_project_start[26], NULL, 0, NULL);
			last_state[26] = 1;
		}
		else
		{
			if (last_state[26] == 1) 
			{
				EnterCriticalSection(&m_csGDILock);
				LIGHT_WHITE BUTTON(BUTTON_X+20,BUTTON_Y+20);
				LeaveCriticalSection(&m_csGDILock);
				mciSendString(Sel_project_stop[26], NULL, 0, NULL);
				last_state[26] = 0;
				count[26] = 0;
				return;
			}
		}
	}
}

void __stdcall T27(void *arg)			// 'NUMPAD1'
{ 
	while (1)
	{
		key_state[27] = GetAsyncKeyState(VK_NUMPAD1);
		if (key_state[27] & 0x8000) 
		{
			EnterCriticalSection(&m_csGDILock);
			LIGHT_AQUA BUTTON(BUTTON_X+30,BUTTON_Y+20);
			LeaveCriticalSection(&m_csGDILock);
			Sleep(DELAY);
			mciSendString(Sel_project_start[27], NULL, 0, NULL);
			last_state[27] = 1;
		}
		else
		{
			if (last_state[27] == 1) 
			{
				EnterCriticalSection(&m_csGDILock);
				LIGHT_WHITE BUTTON(BUTTON_X+30,BUTTON_Y+20);
				LeaveCriticalSection(&m_csGDILock);
				mciSendString(Sel_project_stop[27], NULL, 0, NULL);
				last_state[27] = 0;
				count[27] = 0;
				return;
			}
		}
	}
}

void __stdcall T28(void *arg)			// 'NUMPAD2'
{ 
	while (1)
	{
		key_state[28] = GetAsyncKeyState(VK_NUMPAD2);
		if (key_state[28] & 0x8000) 
		{
			EnterCriticalSection(&m_csGDILock);
			LIGHT_AQUA BUTTON(BUTTON_X+40,BUTTON_Y+20);
			LeaveCriticalSection(&m_csGDILock);
			Sleep(DELAY);
			mciSendString(Sel_project_start[28], NULL, 0, NULL);
			last_state[28] = 1;
		}
		else
		{
			if (last_state[28] == 1) 
			{
				EnterCriticalSection(&m_csGDILock);
				LIGHT_WHITE BUTTON(BUTTON_X+40,BUTTON_Y+20);
				LeaveCriticalSection(&m_csGDILock);
				mciSendString(Sel_project_stop[28], NULL, 0, NULL);
				last_state[28] = 0;
				count[28] = 0;
				return;
			}
		}
	}
}

void __stdcall T29(void *arg)			// 'NUMPAD3'
{ 
	while (1)
	{
		key_state[29] = GetAsyncKeyState(VK_NUMPAD3);
		if (key_state[29] & 0x8000) 
		{
			EnterCriticalSection(&m_csGDILock);
			LIGHT_AQUA BUTTON(BUTTON_X+50,BUTTON_Y+20);
			LeaveCriticalSection(&m_csGDILock);
			Sleep(DELAY);
			mciSendString(Sel_project_start[29], NULL, 0, NULL);
			last_state[29] = 1;
		}
		else
		{
			if (last_state[29] == 1) 
			{
				EnterCriticalSection(&m_csGDILock);
				LIGHT_WHITE BUTTON(BUTTON_X+50,BUTTON_Y+20);
				LeaveCriticalSection(&m_csGDILock);
				mciSendString(Sel_project_stop[29], NULL, 0, NULL);
				last_state[29] = 0;
				count[29] = 0;
				return;
			}
		}
	}
}

/* ���� �Լ� */
int main(void)
{
	InitializeCriticalSection(&m_csGDILock);		// ������ ����ȭ ����
	
	system("title �� ġ �� �� �� �� �� ��");		// Ÿ��Ʋ ����
	system("mode con: cols=165 lines=35");			// �ܼ�â ũ�� ����
	REMOVECURSOR();									// Ŀ�� ����

	Project_select(1);								// ���α׷� ���� �� ����Ǵ� ������Ʈ ����(���� ����)
	mciSendString("play c:\\Project\\system\\bgm.wav", NULL, 0, NULL);		// BGM ���

	INTERFACE01(); LIGHT_RED INTERFACE01(); GREEN INTERFACE01(); LIGHT_PURPLE INTERFACE01(); LIGHT_AQUA INTERFACE01();
	INTERFACE02();
	getch();

	system("cls");
	system("mode con: cols=170 lines=43");

	MENU();

	return 0;
} 

/* ���� Ű �Է� �� ����Ǵ� ����� ���� �Լ� */
void PROJECT_1(int pr_1_link)
{
	int pr_1__true = 1, pr_1_linkey = NULL;
	int value;
	while (pr_1__true)
	{
		pr_1_linkey = getch();
		switch (pr_1_linkey)
		{
		case 0x1B:		// ESC
			mciSendString("stop c:\\Project\\system\\button.wav", NULL, 0, NULL);
			mciSendString("play c:\\Project\\system\\button.wav", NULL, 0, NULL);
			system("cls");
			if(pr_1_link == 0)
			{
				MENU();
				pr_1__true--;
				break;
			}
			if(count_record == 1)		// ���� ���� ��
				remove("test.bin");
			PLAY_LP(0);
			pr_1__true--;
			break;
		case 0x4d:		// ->
			if(pr_1_link == 0)
			{
				pr_1__true--;
				mciSendString("stop c:\\Project\\system\\button.wav", NULL, 0, NULL);
				mciSendString("play c:\\Project\\system\\button.wav", NULL, 0, NULL);
			}
			break;
		case 0x71:		// q
			if (count[0] == 0)
			{
				_beginthreadex(NULL, 0, T0, 0, 0, NULL);
				count[0] = 1;
				if(pr_1_link == 2)
					pr_1__true--;
			}
			break;
		case 0x77:		//w
			if (count[1] == 0)
			{
				_beginthreadex(NULL, 0, T1, 0, 0, NULL);
				count[1] = 1;
				if(pr_1_link == 3)
					pr_1__true--;
			}
			break;
		case 0x65:		//e
			if (count[2] == 0)
			{
				_beginthreadex(NULL, 0, T2, 0, 0, NULL);
				count[2] = 1;
				if(pr_1_link == 4)
					pr_1__true--;
			}
			break;
		case 0x72:		//r
			if (count[3] == 0)
			{
				_beginthreadex(NULL, 0, T3, 0, 0, NULL);
				count[3] = 1;
			}
			break;
		case 0x74:		//t
			if (count[4] == 0)
			{
				_beginthreadex(NULL, 0, T4, 0, 0, NULL);
				count[4] = 1;
			}
			break;
		case 0x79:		//y
			if (count[5] == 0)
			{
				_beginthreadex(NULL, 0, T5, 0, 0, NULL);
				count[5] = 1;
			}
			break;
		case 0x75:		//u
			if (count[6] == 0)
			{
				_beginthreadex(NULL, 0, T6, 0, 0, NULL);
				count[6] = 1;
			}
			break;
		case 0x69:		//i
			if (count[7] == 0)
			{
				_beginthreadex(NULL, 0, T7, 0, 0, NULL);
				count[7] = 1;
			}
			break;
		case 0x6F:		//o
			if (count[8] == 0)
			{
				_beginthreadex(NULL, 0, T8, 0, 0, NULL);
				count[8] = 1;
			}
			break;
		case 0x70:		// p
			if (count[9] == 0)
			{
				_beginthreadex(NULL, 0, T9, 0, 0, NULL);
				count[9] = 1;
			}
			break;
		case 0x61:		// a
			if (count[10] == 0)
			{
				_beginthreadex(NULL, 0, T10, 0, 0, NULL);
				count[10] = 1;
			}
			break;
		case 0x73:		// s
			if (count[11] == 0)
			{
				_beginthreadex(NULL, 0, T11, 0, 0, NULL);
				count[11] = 1;
			}
			break;
		case 0x64:		// d
			if (count[12] == 0)
			{
				_beginthreadex(NULL, 0, T12, 0, 0, NULL);
				count[12] = 1;
			}
			break;
		case 0x66:		// f
			if (count[13] == 0)
			{
				_beginthreadex(NULL, 0, T13, 0, 0, NULL);
				count[13] = 1;
			}
			break;
		case 0x67:		// g
			if (count[14] == 0)
			{
				_beginthreadex(NULL, 0, T14, 0, 0, NULL);
				count[14] = 1;
			}
			break;
		case 0x68:		// h
			if (count[15] == 0)
			{
				_beginthreadex(NULL, 0, T15, 0, 0, NULL);
				count[15] = 1;
			}
			break;
		case 0x6A:		// j
			if (count[16] == 0)
			{
				_beginthreadex(NULL, 0, T16, 0, 0, NULL);
				count[16] = 1;
			}
			break;
		case 0x6B:		// k
			if (count[17] == 0)
			{
				_beginthreadex(NULL, 0, T17, 0, 0, NULL);
				count[17] = 1;
			}
			break;
		case 0x6C:		// l
			if (count[18] == 0)
			{
				_beginthreadex(NULL, 0, T18, 0, 0, NULL);
				count[18] = 1;
			}
			break;
		case 0x7A:		// z
			if (count[19] == 0)
			{
				_beginthreadex(NULL, 0, T19, 0, 0, NULL);
				count[19] = 1;
			}
			break;
		case 0x78:		// x
			if (count[20] == 0)
			{
				_beginthreadex(NULL, 0, T20, 0, 0, NULL);
				count[20] = 1;
			}
			break;
		case 0x63:		// c
			if (count[21] == 0)
			{
				_beginthreadex(NULL, 0, T21, 0, 0, NULL);
				count[21] = 1;
			}
			break;
		case 0x76:		// v
			if (count[22] == 0)
			{
				_beginthreadex(NULL, 0, T22, 0, 0, NULL);
				count[22] = 1;
			}
			break;
		case 0x62:		// b
			if (count[23] == 0)
			{
				_beginthreadex(NULL, 0, T23, 0, 0, NULL);
				count[23] = 1;
			}
			break;
		case 0x6E:		// n
			if (count[24] == 0)
			{
				_beginthreadex(NULL, 0, T24, 0, 0, NULL);
				count[24] = 1;
			}
			break;
		case 0x6D:		// m
			if (count[25] == 0)
			{
				_beginthreadex(NULL, 0, T25, 0, 0, NULL);
				count[25] = 1;
			}
			break;
		case 0x30:		// NUMPAD0
			if (count[26] == 0)
			{
				_beginthreadex(NULL, 0, T26, 0, 0, NULL);
				count[26] = 1;
				if(pr_1_link == 1)
					pr_1__true--;
			}
			break;
		case 0x31:		// NUMPAD1
			if (count[27] == 0)
			{
				_beginthreadex(NULL, 0, T27, 0, 0, NULL);
				count[27] = 1;
			}
			break;
		case 0x32:		// NUMPAD2
			if (count[28] == 0)
			{
				_beginthreadex(NULL, 0, T28, 0, 0, NULL);
				count[28] = 1;
			}
			break;
		case 0x33:		// NUMPAD3
			if (count[29] == 0)
			{
				_beginthreadex(NULL, 0, T29, 0, 0, NULL);
				count[29] = 1;
			}
			break;
		case 61:	// F3
			if(pr_1_link == 11)
			{
				START_RECORD(2);
				pr_1__true--;
			}
			if(pr_1_link == 0)
			{
				TUTO_1();
				pr_1__true--;
			}
			break;
		case 60 :	// F2
			if(pr_1_link == 10)
			{
			START_RECORD(1);
			pr_1__true--;
			}
			break;
		case 59 :	//F1
			if(pr_1_link == 0 || pr_1_link == 11)
			{
			START_RECORD(0);
			pr_1__true--;
			}
			break;

		}
	}
	return;
}

/* �������̽� 1 */
void INTERFACE01(void)
{
	GOTOXY(1,6);
	printf("=====================================================================================================================================================================\n");
	printf("                                                           \n");
	printf("  ����������           ����                                  ����                                  ����  ����   ������������������\n");
	printf("  ��              ��           ��  ��                                  ��  ��                                  ��  ��  ��  ��   ��                              ��\n");
	printf("  �������    ��           ��  ��              �����            ��  ��   ���������������� ��  ��  ��  ��   ��    ���������������\n");
	printf("            ��    ��           ��  ��              ��    ��            ��  ��   ��                          �� ��  ��  ��  ��   ��    ��\n");
	printf("            ��    ��           ��  ��      ������    �������  ��  ��   ����    ������    ���� ��  ��  ��  ��   ��    ��\n");
	printf("  �������    �� �������  ��      ��                      ��  ��  ��       ��    ��      ��    ��     ��  ����  ��   ��    ���������������\n");
	printf("  ��	          �� ��            ��      ������      ������  ��  ��   ����    ������    ���� ��          ��   ��                              ��\n");
	printf("  ��  �������� �������  ��              ��  ��  ��          ��  ��   ��                          �� ��  ����  ��   ������������������\n");
	printf("  ��  ��                       ��  ��            ��  ��  ��  ��        ��  ��   ���������������� ��  ��  ��  ��\n");
	printf("  ��  ��������           ��  ��          ��  ��      ��  ��      ��  ��                                  ��  ��  ��  ��   ������������������\n");
	printf("  ��              ��           ��  ��        ��  ��          ��  ��    ��  ��                                  ��  ��  ��  ��   ��                              ��\n");
	printf("  ����������           ��  ��        ���              ���    ��  ��                                  ��  ��  ��  ��   ������������������\n");
	printf("                               ����                                  ��  ��                                  ����  ����\n");	
	printf("            ����                                                     ��  ��\n");	
	printf("            ��  ��                                                     ����\n");	
	printf("            ��  ����������\n");	                     
	printf("            ��                  ��\n");	
	printf("            ������������\n");	           
	printf("\n====================================================================================================================================================================\n");
	printf("                                                                         L o a d i n g . . . ");
	Sleep(700);
	system("cls");
}
/* �������̽� 2 */
void INTERFACE02(void)
{	
	GOTOXY(1,7);
	LIGHT_WHITE printf("          ������   ��       ���      ��  ������� ��  ��   �������\n");
	LIGHT_WHITE printf("                  ��   ��   �������  ��    ��    ��   ��  ��   ��\n");
	LIGHT_WHITE printf("          ������ ���       ���      ��    ��    ��   ����   ��\n");
	LIGHT_WHITE printf("          ��           ��      ��  ��     ��  ������� ��  ��   �������\n");
	LIGHT_WHITE printf("          ������   ��     ��    ��    ��               ��  ��\n");
	LIGHT_WHITE printf("                ��                        ��                       ��������\n");
	LIGHT_WHITE printf("                ������\n");
	LIGHT_WHITE printf("                                                                                                                                                                    \n");
	printf("                                                                                                                                                                    \n");
	printf("                                                                                                                                                                    \n");
	LIGHT_YELLOW printf("                                                                                                                    ���    ���     ����    �����\n");
	LIGHT_YELLOW printf("                                     ����  ����              ��                                                ��  ��      ��    ��  ��          ��\n");
	LIGHT_YELLOW printf("                                     ��  ��  ��  ��              ��                    ��         ��    ��  ��      ���       ��    ����    �����    \n");
	LIGHT_YELLOW printf("                                     ����  ����  ����  ����  ��  ��   ���  �����      ��    ����   ������    ��   �����   ��\n");
	LIGHT_YELLOW printf("                                     ��      ���    ��  ��  ��  ��  ��  ��  ��        ��         ���    ��       ��  ��      ��      ��      ��\n");
	LIGHT_YELLOW printf("                                     ��      ��  ��  ����  ����   ���    ���     ���       ���    ��       ��  ��    ����    ��      �����\n");
	printf("\n\n\n\n\n\n");
	LIGHT_WHITE printf("                                                                  Press any key when you are ready!!");
}

/* �޴� �Լ� */
void MENU()
{
	int mu_i = 0, mu_key = NULL, mu_true = 1;

	mciSendString("play c:\\Project\\system\\bgm.wav", NULL, 0, NULL);
	LIGHT_WHITE
	printf("                    ___________________________________________________________________________________________________________________________________ \n");
	printf("                   /��������������������������������������������������������������������������������������������������������������������������������  /��\n");
	printf("                  ������������������������������������������������������������������������������������������������������������������������������������:��\n");
    printf("                  ��                               __                                                                                               ��:��\n");                                   
    printf("                  ��               __________     /:/��          ____         __                        __    __      ______________                ��:��\n");                      
    printf("                  ��              /�������� /��   ��:��    _____/�� /____    /:/��    ______________   /:/�� /:/��   /������������ /��              ��:��\n");
	printf("                  ��              ������:��   ��:��   /���������� /��  ��:��   /������������ /�� ��:�� ��:��   ��������/                ��:��\n");
	printf("                  ��               _______��:��   ��:��   ��������/    ��:��   ��������/   ��:��_��:��   ��:|                           ��:��\n");
    printf("                  ��              /������ ��:��   ��:��        ���:��       ��:��     ��:��    ��:��  ��: ����:��   ��:|                           ��:��\n");
	printf("                  ��              ������/   ���:��       ��:����:��     ��:��    _��:��___ ��:��  �����:��   ��:|___________                ��:��\n");
    printf("                  ��              ��:��           ��:��      ��:��  ��:��    ��:��   /:�� /����/��:/�� ��:�� ��:��   ��/���������� /��              ��:��\n");
    printf("                  ��              ��:��______     ��:��     ��:��    ��:��   ��:��   ��������/   ��:�� ��:��   ��������/                ��:��\n");                               
	printf("                  ��              ��/������ /��   ��:��    ��:��      ��:��  ��:��                     ��/   ��/                                    ��:��\n");
	printf("                  ��              ������/     ��/     ��_/         ��_/  ��/                                   ____________________             ��:��\n");
    printf("                  ��                    __                                                                        /������������������ /��           ��:��\n");
    printf("                  ��                   /:/��                                                                      �����������/             ��:��\n");              
	printf("                  ��                   ��:��________                                                                                                ��:��\n");
	printf("                  ��                   ��/�������� /��                                                                                              ��:��\n");
	printf("                  ��                   �������/                                                                                                ��:��\n");
	printf("                  ��                                                                                                                                ��:��\n");
	printf("                  ������������������������������������������������������������������������������������������������������������������������������������ / \n\n");
	printf("��������������������������������������������������������������������������������������������������������������������������������������������������������������������������");
	GOTOXY(1,39);
	printf("��������������������������������������������������������������������������������������������������������������������������������������������������������������������������");

	SELECT_MU(mu_i);		//	�̹��� ���

	GOTOXY(140,41);
	LIGHT_WHITE printf("- product by �� 1 �� 2");

		while (mu_true) 
		{
			mu_key = getch();
			switch (mu_key)
			{
			case 72:		// ����Ű ��
				mu_i--;
				if (mu_i < 0)
					mu_i=2;
				SELECT_MU(mu_i);
				break;
			case 80:		// ����Ű �Ʒ�
				mu_i++;
				if (mu_i > 2) 
					mu_i=0;
				SELECT_MU(mu_i);
				break;
			case 13:		// ����
				if (mu_i == 0)		// ��ġ�е� ����
				{
					mciSendString("stop c:\\Project\\system\\button.wav", NULL, 0, NULL);
					mciSendString("play c:\\Project\\system\\button.wav", NULL, 0, NULL);
					system("cls");
					PLAY_LP(0);
					mu_true--;
				}
				if (mu_i == 1)		// �������� �ҷ�����
				{
					mciSendString("stop c:\\Project\\system\\button.wav", NULL, 0, NULL);
					mciSendString("play c:\\Project\\system\\button.wav", NULL, 0, NULL);
					system("cls");
					RECORD_LOAD();
					mu_true--;
				}
				if (mu_i == 2)		// �����ϱ�
				{
					mciSendString("stop c:\\Project\\system\\button.wav", NULL, 0, NULL);
					mciSendString("play c:\\Project\\system\\button.wav", NULL, 0, NULL);
					printf("\n\n");
					exit(1);
				}
				break;
			}
		}
}

/* �޴� ���� �������̽� */
void SELECT_MU(int sel_mu_link)
{
	int g;
	int sel_mu__x = 40;
	int sel_mu__y[] = {25,26,27, 30,31,32, 35,36,37};

	char p[9][100] =  { ("                            ������ ������ ������ ������ ������"),
						("                            ������   ��   ������ ������   ��  "),
						("                            ������   ��   ��  �� ������   ��  "),
					    ("                    ��    ������������ ����   ������ ������ ��     ������"),
						("                    ��    ��  ���������� ��   ������   ��   ��     ������"),
						("                    ��������������  �� ����   ��     ������ ������ ������"),
						("                             ������  ��������  ������  ������"),
						("                             ������    ����      ��      ��  "),
						("                             ������  ��������  ������    ��  "),
					};
	char *q[]= {p, p+1, p+2, p+3, p+4, p+5, p+6, p+7, p+8};

	if(sel_mu_link == 0)
	{
		for(g=0; g<9; g++)			// ��ü �������̽� ���
		{
			GOTOXY(sel_mu__x,sel_mu__y[g]);
			LIGHT_WHITE printf("%s", q[g]);
		}
		for(g=0; g<3; g++)			// start�� ��������� ���
		{
			GOTOXY(sel_mu__x-1,sel_mu__y[g]);
			LIGHT_YELLOW printf("%s", q[g]);
		}
		GOTOXY(sel_mu__x+6,sel_mu__y[1]); printf("������");
	}
	else if(sel_mu_link == 1)
	{
	for(g=0; g<9; g++)			// ��ü �������̽� ���
		{
			GOTOXY(sel_mu__x,sel_mu__y[g]);
			LIGHT_WHITE printf("%s", q[g]);
		}
		for(g=3; g<6; g++)			// load file�� ��������� ���
		{
			GOTOXY(sel_mu__x-1,sel_mu__y[g]);
			LIGHT_YELLOW printf("%s", q[g]);
		}
		GOTOXY(sel_mu__x+6,sel_mu__y[4]); printf("������");
	}
	else if(sel_mu_link == 2)
	{
		for(g=0; g<9; g++)			// ��ü �������̽� ���
		{
			GOTOXY(sel_mu__x,sel_mu__y[g]);
			LIGHT_WHITE printf("%s", q[g]);
		}
		for(g=6; g<9; g++)			// exit�� ��������� ���
		{
			GOTOXY(sel_mu__x-1,sel_mu__y[g]);
			LIGHT_YELLOW printf("%s", q[g]);
		}
		GOTOXY(sel_mu__x+6,sel_mu__y[7]); printf("������");
	}

}

/* ��ġ�е� ���� �Լ� */
void PLAY_LP(int pl_link)
{
	mciSendString("stop c:\\Project\\system\\bgm.wav", NULL, 0, NULL);
	if(pl_link == 0)		// �ܼ� ���� ���
	{
		LIGHT_WHITE
		printf("\n\n\n                                                                                                F1 ���� ����                F3 Ʃ�丮��                Esc ������\n");
		GOTOXY(1,3);
		LIGHT_RED 
		printf("    ��    ��������  ��������������    ������������ ����\n");
		printf("    ��    ��������  ����  ����  �������������������� ��\n");
		printf("    ��������  ����������  ��������  ����    ��  �� ����\n");
	}
	if(pl_link == 1 || pl_link == 2)		// �ܼ� ���� ���
	{
		LIGHT_WHITE
		printf("\n\n\n                                                                                                                                               Esc Ʃ�丮�� ����\n");
		GOTOXY(1,3);
		LIGHT_RED
		printf("    ������                                  ��\n");
		printf("      ��   ��  ��  ��   ������    ������    ��\n");
		printf("      ��   ��  ���������� ������������  ��  ��\n");
		printf("      ��   ������  ��  ���� ��    ��  ��������\n");
	}
	if(pl_link == 0 || pl_link == 1 || pl_link == 2)		// �ܼ� ���� ���
	{
		GOTOXY(139,10); LIGHT_WHITE printf("��");
		GOTOXY(67,10);GREEN printf("��");
		GOTOXY(1,8);
		LIGHT_WHITE printf("=========================================================================================================================================================================\n\n\n\n");
		printf("                                    ��----------------------------------------------------------��                   ��\n");
		printf("                                    ��������������������������������������                   ��           ������Ʈ ���� ����\n");
		printf("                                    ��������������������������������������                   ��\n");
		printf("                                    ��������������������������������������                   ��\n");
		printf("                                    ��������������������������������������                   ��               Project 1\n");
		printf("                                    ��----------------------------------------------------------��                   ��\n");
		printf("                                    ��������������������������������������                   ��\n");
		printf("                                    ��������������������������������������                   ��               Project 2\n");
		printf("                                    ��������������������������������������                   ��\n");
		printf("                                    ��������������������������������������                   ��\n");
		printf("                                    ��----------------------------------------------------------��                   ��               Project 3\n");
		printf("                                    ��������������������������������������                   ��\n");
		printf("                                    ��������������������������������������                   ��\n");
		printf("                                    ��������������������������������������                   ��               Project 4\n");
		printf("                                    ��������������������������������������                   ��\n");
		printf("                                    ��----------------------------------------------------------��                   ��\n");
		printf("                                    ��������������������������������������                   ��               Project 5\n");
		printf("                                    ��������������������������������������                   ��\n");
		printf("                                    ��������������������������������������                   ��\n");
		printf("                                    ��������������������������������������                   ��\n");
		printf("                                    ��----------------------------------------------------------��                   ��\n");
		printf("                                    ��������������������������������������                   ��\n");
		printf("                                    ��������������������������������������                   ��\n");
		printf("                                    ��������������������������������������                   ��\n");
		printf("                                    ��������������������������������������                   ��\n");
		printf("                                    ��----------------------------------------------------------��                   ��\n");;
		GOTOXY(140,41);
		printf("- product by �� 1 �� 2");
	}
	
	if(pl_link == 2)		// Ʃ�丮���� ���弱
		TUTO_2();

	if(pl_link == 0)		// ���� �� �������̽� ���
	{
		PROJECT_1(0);
		SELECT_LP_1(1, 0);
	}
}

/* ��ġ�е� �۵� �Լ� */
void SELECT_LP_1(int sel_lp_1_link_1, int sel_lp_1_link_2)
{
	int sel_lp_1__true = 1, sel_lp_1__key = NULL;

	while(sel_lp_1__true)
	{
		SELECT_LP_2(sel_lp_1_link_1);
		sel_lp_1__key = getch();
		switch(sel_lp_1__key)
		{
		case 61:				// F3
			if(sel_lp_1_link_2 == 0)
			{
				TUTO_1();
				sel_lp_1__true--;
			}
			break;
		case 0x1B:				// Esc
			mciSendString("stop c:\\Project\\system\\button.wav", NULL, 0, NULL);
			mciSendString("play c:\\Project\\system\\button.wav", NULL, 0, NULL);
			system("cls");
			if(sel_lp_1_link_2 == 0){
				MENU();
			}
			if(sel_lp_1_link_2 == 1){
				PLAY_LP(0);
			}
			break;
		case 75:				// <-
			mciSendString("stop c:\\Project\\system\\button.wav", NULL, 0, NULL);
			mciSendString("play c:\\Project\\system\\button.wav", NULL, 0, NULL);
			SELECT_LP_2(0);
			PROJECT_1(0);
			break;
		case 80:				//  �Ʒ�
			mciSendString("stop c:\\Project\\system\\button2.wav", NULL, 0, NULL);
			mciSendString("play c:\\Project\\system\\button2.wav", NULL, 0, NULL);
			sel_lp_1_link_1++;
			if(sel_lp_1_link_1>5)
				sel_lp_1_link_1=1;
			SELECT_LP_2(sel_lp_1_link_1);
			break;
		case 72:				// ��
			mciSendString("stop c:\\Project\\system\\button2.wav", NULL, 0, NULL);
			mciSendString("play c:\\Project\\system\\button2.wav", NULL, 0, NULL);
			sel_lp_1_link_1--;
			if(sel_lp_1_link_1<1)
				sel_lp_1_link_1=5;
			SELECT_LP_2(sel_lp_1_link_1);
			break;
		case 13:				// Enter, project ���� ���(project 1)
			if(sel_lp_1_link_1 == 1)			
			{
				mciSendString("stop c:\\Project\\system\\project1.wav", NULL, 0, NULL);
				mciSendString("play c:\\Project\\system\\project1.wav", NULL, 0, NULL);
				system("cls");
				if(sel_lp_1_link_2 == 0){
					Project_select(sel_lp_1_link_1);
					PLAY_LP(0);
				}
				if(sel_lp_1_link_2 == 1)
					PLAY_LP(2); // Ʃ�丮��κ�
				sel_lp_1__true--;
			}
			if(sel_lp_1_link_1 == 2)			// project 2
			{
				mciSendString("stop c:\\Project\\system\\project1.wav", NULL, 0, NULL);
				mciSendString("play c:\\Project\\system\\project1.wav", NULL, 0, NULL);
				if(sel_lp_1_link_2 == 0)
				{
					Project_select(sel_lp_1_link_1);
					PLAY_LP(0);
					sel_lp_1__true--;
				}
			}
			if(sel_lp_1_link_1 == 3)			// project 3
			{
				mciSendString("stop c:\\Project\\system\\project1.wav", NULL, 0, NULL);
				mciSendString("play c:\\Project\\system\\project1.wav", NULL, 0, NULL);
				if(sel_lp_1_link_2 == 0)
				{
					Project_select(sel_lp_1_link_1);
					PLAY_LP(0);
					sel_lp_1__true--;
				}
			}
			if(sel_lp_1_link_1 == 4)			// project 4
			{
				mciSendString("stop c:\\Project\\system\\project1.wav", NULL, 0, NULL);
				mciSendString("play c:\\Project\\system\\project1.wav", NULL, 0, NULL);
				if(sel_lp_1_link_2 == 0)
				{
					Project_select(sel_lp_1_link_1);
					PLAY_LP(0);
					sel_lp_1__true--;
				}
			}
			if(sel_lp_1_link_1 == 5)			// project 5
			{
				mciSendString("stop c:\\Project\\system\\project1.wav", NULL, 0, NULL);
				mciSendString("play c:\\Project\\system\\project1.wav", NULL, 0, NULL);
				if(sel_lp_1_link_2 == 0)
				{
					Project_select(sel_lp_1_link_1);
					PLAY_LP(0);
					sel_lp_1__true--;
				}
			}
			break;
		}
	}
	return;
}
/* ������Ʈ �������̽� ��� �Լ� */
void SELECT_LP_2(int sel_lp_2_link)
{
	int g;
	int sel_lp_2__x = 129;
	int sel_lp_2__y[] = {16, 19, 22, 25, 28};

	char p[5][30] = {   ("      Project 1\n\n\n"),
						("      Project 2\n\n\n"),
						("      Project 3\n\n\n"),
						("      Project 4\n\n\n"),
						("      Project 5\n\n\n"),
					};
	char *q[]={p, p+1, p+2, p+3, p+4};


	if(sel_lp_2_link == 0)		//		�� �ʷϻ����� �ٲ�
	{
		GOTOXY(139,10); LIGHT_WHITE printf("��");
		GOTOXY(67,10); GREEN printf("��");

		for(g=0; g<5; g++)		//		��ü ���
		{
			GOTOXY(sel_lp_2__x,sel_lp_2__y[g]);
			LIGHT_WHITE printf("%s", q[g]);
		}
	}
	if(sel_lp_2_link == 1)		//		�� �Ͼ������ �ٲ�
	{
		GOTOXY(139,10); GREEN printf("��");
		GOTOXY(67,10); LIGHT_WHITE printf("��");

		for(g=0; g<5; g++)		//		project 1�� ��������� ���
		{
			GOTOXY(sel_lp_2__x,sel_lp_2__y[g]);
			LIGHT_WHITE printf("%s", q[g]);
		}
		GOTOXY(sel_lp_2__x-1,sel_lp_2__y[0]); LIGHT_YELLOW printf(" %s", q[0]);
		GOTOXY(sel_lp_2__x,sel_lp_2__y[0]);  printf("  ��");
	}
	if(sel_lp_2_link == 2)
	{
		GOTOXY(139,10); GREEN printf("��");
		GOTOXY(67,10); LIGHT_WHITE printf("��");

		for(g=0; g<5; g++)		//		project 2�� ��������� ���
		{
			GOTOXY(sel_lp_2__x,sel_lp_2__y[g]);
			LIGHT_WHITE printf("%s", q[g]);
		}
		GOTOXY(sel_lp_2__x-1,sel_lp_2__y[1]); LIGHT_YELLOW printf(" %s", q[1]);
		GOTOXY(sel_lp_2__x,sel_lp_2__y[1]); printf("  ��");
	}
	if(sel_lp_2_link == 3)
	{
		GOTOXY(139,10); GREEN printf("��");
		GOTOXY(67,10); LIGHT_WHITE printf("��");

		for(g=0; g<5; g++)		//		project 3�� ��������� ���
		{
			GOTOXY(sel_lp_2__x,sel_lp_2__y[g]);
			LIGHT_WHITE printf("%s", q[g]);
		}
		GOTOXY(sel_lp_2__x-1,sel_lp_2__y[2]); LIGHT_YELLOW printf(" %s", q[2]);
		GOTOXY(sel_lp_2__x,sel_lp_2__y[2]); printf("  ��");
	}
	if(sel_lp_2_link == 4)
	{
		GOTOXY(139,10); GREEN printf("��");
		GOTOXY(67,10); LIGHT_WHITE printf("��");

		for(g=0; g<5; g++)		//		project 4�� ��������� ���
		{
			GOTOXY(sel_lp_2__x,sel_lp_2__y[g]);
			LIGHT_WHITE printf("%s", q[g]);
		}
		GOTOXY(sel_lp_2__x-1,sel_lp_2__y[3]); LIGHT_YELLOW printf(" %s", q[3]);
		GOTOXY(sel_lp_2__x,sel_lp_2__y[3]); printf("  ��");
	}
	if(sel_lp_2_link == 5)
	{
		GOTOXY(139,10); GREEN printf("��");
		GOTOXY(67,10); LIGHT_WHITE printf("��");

		for(g=0; g<5; g++)		//		project 5�� ��������� ���
		{
			GOTOXY(sel_lp_2__x,sel_lp_2__y[g]);
			LIGHT_WHITE printf("%s", q[g]);
		}
		GOTOXY(sel_lp_2__x-1,sel_lp_2__y[4]); LIGHT_YELLOW printf(" %s", q[4]);
		GOTOXY(sel_lp_2__x,sel_lp_2__y[4]); printf("  ��");
	}
}

/* ���� ���� �ҷ����� */
void RECORD_LOAD(void)
{
 
 HANDLE hSearch;
 int i=0, j=0, record_count=0;
 char key;
 char file_name[100]={0};
 int first_page=0, last_page=10;
 int cursor=0;

 int x=75;
 int y[]={14, 16, 18, 20, 22, 24, 26, 28, 30, 32};

 BOOL fFinished = TRUE; 

 system("cls");

 LIGHT_WHITE
 printf("\n\n                                                                 ��� �������ϼ���      ��� ����������      Enter �����������      Space �������      Esc ������\n");
 GOTOXY(66,6); printf(" F3 �������� �˻�");
 GOTOXY(1,3);
 LIGHT_RED 
 printf("    ��    ��������  ��������������    ������������ ����\n");
 printf("    ��    ��������  ����  ����  �������������������� ��\n");
 printf("    ��������  ����������  ��������  ����    ��  �� ����\n");


GOTOXY(1,8);
LIGHT_WHITE printf("=========================================================================================================================================================================\n\n\n");



 mciSendString("stop c:\\Project\\system\\bgm.wav", NULL, 0, NULL);
 hSearch = FindFirstFile("c:\\Project\\record\\*.wav", &FileData[0]); // �ش��ο� ������ ������� �ڵ鰪 ��ȯ, �ƴҰ�� INVALID_HANDLE_VALUE ��ȯ
 if (hSearch == INVALID_HANDLE_VALUE){
	 LIGHT_WHITE printf("���������� �����ϴ�\nESCŰ�� ������ ���θ޴��� ���ư��ϴ�.\n");
	 key=getch();
	 if(key==0x1b){
		 system("cls");
		 MENU();
		 return;
	 }
	 else{
		 system("cls");
		 i--;
	 }
 }
 // ���� �˻�����
 i=0;
 while (fFinished) //�ʱⰪ TRUE 
 {
	 //LIGHT_WHITE printf("%s\n", FileData[i].cFileName);
	 if (!FindNextFile(hSearch, &FileData[i+1])) // FindNextFile �̿��� ������ �˻���(������ ������� 0�̾ƴѰ� ��ȯ, ������� 0��ȯ)
		 // FindNextFile �Լ��� ������ ���� ã�Ƽ� 0�� ��ȯ�� ��� if���� �����(0�� �ݴ��̱� ������)
	 {
		 if (GetLastError() == ERROR_NO_MORE_FILES) // ������ ���̻� ������ ���ٶ�� ������ ���
		 {
			 record_count=i;
			 fFinished = FALSE; // FALSE�� ���� �ݺ����� Ż����
		 }
	 }
	 i++;
 }

 GOTOXY(70, y[cursor]); LIGHT_YELLOW printf("��");

 while(1){
	 for(i=first_page; i<last_page; i++){
		 GOTOXY(80, 11); LIGHT_WHITE printf("������ %2d/10", (i/10)+1);
		 
		 if(i>record_count){
			 GOTOXY(x, y[j]); LIGHT_WHITE printf("�������� %2d : �󽽷�\t\t\t\t\t\t\t\t\t\n", i+1);
			 j++;
			 if(j==10) j=0;
			 continue;
		 }
		GOTOXY(x, y[j]); LIGHT_WHITE printf("�������� %2d : %s\t\t\t\t\t\t\t\t\t\n", i+1, FileData[i].cFileName);
		j++;  
		if(j==10) j=0;
	 }
	 key=getch();
	 switch(key)
	 {
	 case 77:// ����Ű ������
		 first_page+=10;
		 last_page+=10;
		 if(first_page==100){
			 first_page=0;
			 last_page=10;
		 }
		 break;
	 case 75:// ����Ű ����
		 first_page-=10;
		 last_page-=10;
		 if(first_page==-10){
			 first_page=90;
			 last_page=100;
		 }
		 break;
	 case 72:// ����Ű ��
		 cursor--;
		 GOTOXY(70, y[cursor+1]); printf("   ");
		 GOTOXY(70, y[cursor]); LIGHT_YELLOW printf("��");
		 if(cursor<0){
			 GOTOXY(70, y[0]); printf("    ");
			 GOTOXY(70, y[9]); LIGHT_YELLOW printf("��");
			 cursor=9;
		 }
		 break;
	 case 80:// ����Ű �Ʒ�
		 cursor++;
		 GOTOXY(70, y[cursor-1]); printf("   ");
		 GOTOXY(70, y[cursor]); LIGHT_YELLOW printf("��");
		 if(cursor>9){
			GOTOXY(70, y[9]); printf("    ");
			GOTOXY(70, y[0]); LIGHT_YELLOW printf("��");
			cursor=0;
		 }
		 break;
	 case 13: // ����
		 if(first_page+cursor>record_count){
			 GOTOXY(79, 35); printf("�󽽷��Դϴ�");
			 break;
		 }
		 sprintf(file_name, "%s%s%s", MUSIC_PLAY, RECORD_LINK, FileData[first_page+cursor].cFileName);
		 GOTOXY(79, 35); printf("\t\t\t\t\t\t\t");
		 mciSendString(file_name, NULL, 0, NULL);
		 break;
	 case 32: // �����̽���
		 if(first_page+cursor>record_count){
			 break;
		 }
		 sprintf(file_name, "%s%s%s", MUSIC_STOP, RECORD_LINK, FileData[first_page+cursor].cFileName);
		 mciSendString(file_name, NULL, 0, NULL);
		 break;
	 case 61: // F3 (�˻�)
		  record_search();
		  return;
	 case 0x1b:// ESC
		 system("cls");
		 MENU();
		 break;
		 return;
 }
 }
}

/* ���� ���� ã�� */
void record_search(){
	char search_name[50];
	char file_name[50];
	int i=0;

	int x=74;
	int y[]={25, 27};
	int cursor=0;

	char key;

	system("cls");

	LIGHT_WHITE
	GOTOXY(1,3);
	LIGHT_RED 
	printf("    ��    ��������  ��������������    ������������ ����\n");
	printf("    ��    ��������  ����  ����  �������������������� ��\n");
	printf("    ��������  ����������  ��������  ����    ��  �� ����\n");
	
	GOTOXY(1,8);
	LIGHT_WHITE printf("=========================================================================================================================================================================\n\n\n");

	
	LIGHT_WHITE GOTOXY(70, 12); printf("�����̸��� �Է����ּ��� : ");
	LIGHT_YELLOW gets(search_name);
		
	sprintf(file_name, "%s%s",search_name, FILE_EXTENSION);
		
	for(i=0; i<100; i++){
		if(!strcmp(file_name, FileData[i].cFileName)){
			break;
		}
	}
	if(i==100){

		LIGHT_WHITE
		GOTOXY(38, 18); printf("��---------------------------------------------------------------------------------------------��\n");
		GOTOXY(38, 19); printf("��                                                                                             ��\n");
		GOTOXY(38, 20); printf("��                                                                                             ��\n");
		GOTOXY(38, 21); printf("��                                                                                             ��\n");
		GOTOXY(38, 22); printf("��                                                                                             ��\n");
		GOTOXY(38, 23); printf("��                                                                                             ��\n");
		GOTOXY(38, 24); printf("��                                                                                             ��\n");
		GOTOXY(38, 25); printf("��                                                                                             ��\n");
		GOTOXY(38, 26); printf("��                                                                                             ��\n");
		GOTOXY(38, 27); printf("��---------------------------------------------------------------------------------------------��\n");

		LIGHT_AQUA GOTOXY(75, 21); printf("ã�� ������ �����ϴ�");
		LIGHT_AQUA GOTOXY(69, 24); printf("Enter�� ������ �޴��� ���ư��ϴ�.");
		while(1){
			key=getch();
			if(key==13){
				RECORD_LOAD();
				return;
			}
			else{
				continue;
			}
		}
	}
	else{
		LIGHT_WHITE
		GOTOXY(38, 18); printf("��---------------------------------------------------------------------------------------------��\n");
		GOTOXY(38, 19); printf("��                                                                                             ��\n");
		GOTOXY(38, 21); printf("��                                                                                             ��\n");
		GOTOXY(38, 22); printf("��---------------------------------------------------------------------------------------------��\n");

		LIGHT_YELLOW GOTOXY(70, 20); printf("��");
		LIGHT_WHITE  GOTOXY(75, 20); printf("�������� %2d : %s\t\t\t\t\t\t\t\t\t\n", i+1, FileData[i].cFileName);

		LIGHT_YELLOW GOTOXY(x, y[cursor]); printf("��");
		LIGHT_WHITE  GOTOXY(80, 25); printf("���");
		LIGHT_WHITE  GOTOXY(80, 27); printf("������");
		

		while(1){
			key=getch();
			switch(key){
			case 72:// ����Ű ��
				cursor--; 
				GOTOXY(x, y[cursor+1]); printf("  ");
				GOTOXY(x, y[cursor]); LIGHT_YELLOW printf("��");
				if(cursor<0){
					GOTOXY(x, y[0]); printf("    ");
					GOTOXY(x, y[1]); LIGHT_YELLOW printf("��");
					cursor=1;
				}
				break;
			case 80:// ����Ű �Ʒ�
				 cursor++;
				 GOTOXY(x, y[cursor-1]); printf("   ");
				 GOTOXY(x, y[cursor]); LIGHT_YELLOW printf("��");
				 if(cursor>1){
					 GOTOXY(x, y[1]); printf("    ");
					 GOTOXY(x, y[0]); LIGHT_YELLOW printf("��");
					 cursor=0;
				 }
				 break;
			case 13: // ����
				if(cursor==0){
				sprintf(file_name, "%s%s%s", MUSIC_PLAY, RECORD_LINK, FileData[i].cFileName);
				mciSendString(file_name, NULL, 0, NULL);
				//Sleep(1000);
				RECORD_LOAD();
				return;
				}
				else{
					RECORD_LOAD();
					return;
				}
			}
		}
	}
}

void BUTTON(int bu_x, int bu_y)
{
	GOTOXY(bu_x, bu_y); printf("�����\n");
	GOTOXY(bu_x, bu_y+1); printf("�����\n");
	GOTOXY(bu_x, bu_y+2); printf("�����\n");
	GOTOXY(bu_x, bu_y+3); printf("�����\n");
}

void GOTOXY(int x, int y)
{
	COORD Pos = { x - 1, y - 1 };
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), Pos);
}

void REMOVECURSOR(void)

{

 CONSOLE_CURSOR_INFO stCur = { 1, FALSE };

 SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &stCur);

}

/* Ʃ�丮�� �Լ� 1 */
void TUTO_1()
{
	int tu_1_key, tu_1_true;
	tu_1_key = 0;
	tu_1_true = 1;

	system("cls");
	PLAY_LP(1);

	LIGHT_WHITE GOTOXY(59,4); printf("��"); printf("           �� ���ڰ� �ʷϻ��� ��Ʈ��       �˴ϴ�        "); printf("��");
	GREEN GOTOXY(68,4); printf("��"); LIGHT_YELLOW GOTOXY(98,4); printf("Ȱ��ȭ"); 
	LIGHT_WHITE GOTOXY(59,5); printf("��"); printf("                      �ϱ�����              �� ��������  "); printf("��");
	LIGHT_YELLOW GOTOXY(63,5); printf(" Project File�� ����"); GOTOXY(91,5); printf(" ����Ű ������");
	LIGHT_WHITE GOTOXY(59,6); printf("��"); LIGHT_YELLOW printf("                    �� �� �� �� �� �� ��                 "); LIGHT_WHITE printf("��");
	while(tu_1_true)
	{
		tu_1_key = getch();
		if(tu_1_key == 0x4d)			// ->
		{
			mciSendString("stop c:\\Project\\system\\button.wav", NULL, 0, NULL);
			mciSendString("play c:\\Project\\system\\button.wav", NULL, 0, NULL);
			system("cls");
			PLAY_LP(1);
			LIGHT_AQUA
			LIGHT_WHITE GOTOXY(59,4); printf("��"); printf("                        �ؼ� ������Ʈ ������      ��       "); printf("��");
			LIGHT_YELLOW GOTOXY(72,4); printf("����Ű�� ����"); GOTOXY(106,4); printf("����");
			LIGHT_WHITE GOTOXY(59,5); printf("��"); printf("                ���ڸ�         ��ų�� �ֽ��ϴ�             "); printf("��");
			GREEN GOTOXY(74,5); printf("��"); GOTOXY(85,5); LIGHT_YELLOW printf("Ȱ��ȭ");
			LIGHT_WHITE GOTOXY(59,6); printf("��"); printf("                                  �غ�����                 "); printf("��");
			LIGHT_YELLOW GOTOXY(79,6); printf("Project 1�� ����");
			SELECT_LP_1(1, 1);
			tu_1_true--;
		}
		if(tu_1_key == 0x1B)			// esc
		{
			mciSendString("stop c:\\Project\\system\\button.wav", NULL, 0, NULL);
			mciSendString("play c:\\Project\\system\\button.wav", NULL, 0, NULL);
			system("cls");
			PLAY_LP(0);
			tu_1_true--;
		}
	}
}

/* Ʃ�丮�� �Լ� 2 */
void TUTO_2()
{
	int tu_2_value=1,tu_2_true=1;

	while(tu_2_true)		// ���������� 5�� ���� " tu_2_value++ "�� �̿�
	{
		if(tu_2_value == 1)
		{
			EnterCriticalSection(&m_csGDILock);
			LIGHT_WHITE GOTOXY(68,6); printf("�� '          '�� ��� ����������! ��");
			LIGHT_YELLOW GOTOXY(73,6); printf("Numpad 0");
			LeaveCriticalSection(&m_csGDILock);
			PROJECT_1(tu_2_value);			// ���ǿ� �´� ���� ���
			tu_2_value++;
		}
		if(tu_2_value == 2)
		{
			Sleep(400);
			EnterCriticalSection(&m_csGDILock);
			LIGHT_WHITE GOTOXY(68,6); printf("�� '       '�� ��� ����������!   ��");
			LIGHT_YELLOW GOTOXY(73,6); printf("Key Q");
			LeaveCriticalSection(&m_csGDILock);
			PROJECT_1(tu_2_value);
			tu_2_value++;
		}
		if(tu_2_value == 3)
		{
			Sleep(400);
			EnterCriticalSection(&m_csGDILock);
			LIGHT_WHITE GOTOXY(68,6); printf("�� '       '�� ��� ����������!   ��");
			LIGHT_YELLOW GOTOXY(73,6); printf("Key W");
			LeaveCriticalSection(&m_csGDILock);
			PROJECT_1(tu_2_value);
			tu_2_value++;
		}

		if(tu_2_value == 4)
		{
			Sleep(400);
			EnterCriticalSection(&m_csGDILock);
			LIGHT_AQUA
			LIGHT_WHITE GOTOXY(68,6); printf("�� '       '�� ��� ����������!   ��");
			LIGHT_YELLOW GOTOXY(73,6); printf("Key E");
			LeaveCriticalSection(&m_csGDILock);
			PROJECT_1(tu_2_value);
			tu_2_true--;
		}
	}
	Sleep(400);
	EnterCriticalSection(&m_csGDILock);
	LIGHT_WHITE GOTOXY(68,6); printf("                                    ");
	GOTOXY(68,6); printf("��     ���丮���� ��ġ�ڽ��ϴ�    ��");
	GOTOXY(68,7); printf("��        �Է� �� ���丮�� ����!  ��");
	GOTOXY(74,7); LIGHT_AQUA printf("ESC");
	LeaveCriticalSection(&m_csGDILock);
	PROJECT_1(5);
}
 
/* ���� ���� */
void START_RECORD(int record_link)
{
	HWAVEIN hWaveIn;		// HANDLE���� ����
	MMRESULT result = 0;
	WAVEFORMATEX pFormat;	// ����ü ����
	WAVE_HEADER header;		// wav���� ���

	FILE *fp = NULL;
	FILE *bin = NULL;

	const int NUMPTS = sampleRate * duration * channel * (int)(bps / 8);   // signal length
	static time_t start, end;		// ���ϸ� �ȵǱ� ������ (static)
	double pst = 0.0;
	int i, bytes, record_stage;
	char file_name[20];
	char file_name_wav[22];

	pFormat.wFormatTag = WAVE_FORMAT_PCM; // simple, uncompressed format
	pFormat.nChannels = channel; // 1=mono, 2=stereo
	pFormat.nSamplesPerSec = sampleRate; // 44100
	pFormat.nAvgBytesPerSec = sampleRate * (int)(bps / 8) * channel;   // = nSamplesPerSec * n.Channels * wBitsPerSample/8
	pFormat.nBlockAlign = channel * (int)(bps/8); // = n.Channels * wBitsPerSample/8
	pFormat.wBitsPerSample = bps; // 16 for high quality, 8 for telephone-grade
	pFormat.cbSize = 0;
 
// Specify recording parameters
 
	result = waveInOpen(&hWaveIn, WAVE_MAPPER, &pFormat, 0L, 0L, WAVE_FORMAT_DIRECT);		// �����ϱ� ���� �������̹��� HANDLE���� ��ȯ

// Set up an prepare header for input

	WaveInHdr.lpData = (char *)waveIn;
	WaveInHdr.dwBufferLength = NUMPTS * 2;
	WaveInHdr.dwBytesRecorded = 0;
	WaveInHdr.dwUser = 0L;
	WaveInHdr.dwFlags = 0L;
	WaveInHdr.dwLoops = 0L;

	waveInPrepareHeader(hWaveIn, &WaveInHdr, sizeof(WAVEHDR));		// wav���� ����� HANDLE���� �غ������ �˸�
 
// Insert a wave input buffer
	waveInAddBuffer(hWaveIn, &WaveInHdr, sizeof(WAVEHDR));

	if(record_link == 0)		// ���� ����
	{
		start = time(NULL);

		result = waveInStart(hWaveIn);		// ���� ����
		GOTOXY(80,4); LIGHT_WHITE printf("   F1 ���� �簳      F2 ���� ���� / �Ͻ� ����      F3 ���� ���� ����      Esc ������\n");
		GOTOXY(80,4); LIGHT_YELLOW printf("   F1 ���� �簳");
		GOTOXY(55,10);GREEN printf("��  R E C O R D I N G  ��");
		
		PROJECT_1(10);
	}
	if(record_link == 1)		// ���� �Ͻ� ����
	{
		end = time(NULL);
		pst = difftime(end,start);		// ������ �Ͻ� ���� ������ �ð�
		ttt+=pst;						// �� ���� �ð�
		waveInStop(hWaveIn);			// ���� ����

		bin = fopen("c:\\Project\\system\\test.bin", "ab+");		// ���� ���� ���� �� �߰� ���� ���
	
		for (i = 0; i < sampleRate * pst * (int)(bps / 8); i++)		// ������ ���� �������Ͽ� �ۼ�
			fwrite(&waveIn[i], sizeof(short), 1, bin);
		
		fclose(bin);

		count_record = 1;		// esc�� ����� �������� ����
		GOTOXY(80,4); LIGHT_WHITE printf("   F1 ���� �簳");
		GOTOXY(101,4); LIGHT_YELLOW printf("F2 ���� ���� / �Ͻ� ����");
		GOTOXY(55,10);GREEN printf("��      P A U S E      ��");
		if(record_value == 1)			// �Ͻ����� �ϰ� �ٽ� ������ �簳�ϰ� �ٽ� �Ͻ����� ���� ��,
		{
			i=0;
			while(1)
			{
			GOTOXY(49,40); LIGHT_AQUA printf("������ �����ϰ� �ֽ��ϴ�. ��ø� ��ٷ��ּ���.    ");
			Sleep(700);
			GOTOXY(49,40); LIGHT_AQUA printf("������ �����ϰ� �ֽ��ϴ�. ��ø� ��ٷ��ּ���. .  ");
			Sleep(700);
			GOTOXY(49,40); LIGHT_AQUA printf("������ �����ϰ� �ֽ��ϴ�. ��ø� ��ٷ��ּ���. . .");
			Sleep(700);
			if(i == 1)
				break;
			i++;
			}

			GOTOXY(49,40); LIGHT_AQUA printf("                                                  ");
		}
		record_value = 1;
		PROJECT_1(11);
	}

		

	if(record_link == 2)		// ���� ���� ����
	{
		bin = fopen("c:\\Project\\system\\test.bin", "rb");		// �������� �б���� ����

		i=0;

		while(1)		// wavein�迭�� �������� ���� ����
		{
			bytes = fread(&waveIn[i], sizeof(short), 1, bin);
			if(bytes <= 0)
				break;
			i++;
		}

		GOTOXY(101,4); LIGHT_WHITE printf("F2 ���� ���� / �Ͻ� ����");
		GOTOXY(131,4); LIGHT_YELLOW printf("F3 ���� ���� ����");
		GOTOXY(55,40); LIGHT_AQUA printf(" ������ ���� �̸� �Է� : ");
		LIGHT_YELLOW gets(file_name);
		sprintf(file_name_wav, "%s%s%s", RECORD_LINK, file_name, FILE_EXTENSION);		// ���ڿ� ����

		fopen_s(&fp, file_name_wav, "wb");		// wav ���� �������� ������� ����

		/* wav ��� �ʱ�ȭ */
		memcpy(header.Riff.ChunkID, "RIFF", 4);
		header.Riff.ChunkSize = ttt * sampleRate * channel * (int)(bps / 8) + 36;
		memcpy(header.Riff.Format, "WAVE", 4);
 
		memcpy(header.Fmt.ChunkID, "fmt ", 4);
		header.Fmt.ChunkSize = 0x10;
		header.Fmt.AudioFormat = WAVE_FORMAT_PCM;
		header.Fmt.NumChannels = channel;
		header.Fmt.SampleRate = sampleRate;
		header.Fmt.AvgByteRate = sampleRate * channel * (int)(bps / 8);
		header.Fmt.BlockAlign = channel * (int)(bps / 8);
		header.Fmt.BitPerSample = bps;
 
		memcpy(header.Data.ChunkID, "data", 4);
		header.Data.ChunkSize = ttt * sampleRate * channel * (int)(bps / 8);
 
		fwrite(&header, sizeof(header), 1, fp);		// wav ��� ������ ����
 
		for (i = 0; i < sampleRate * ttt * (int)(bps / 8); i++)		// ���� ���� �ڿ� wav ��� ������ ����
			fwrite(&waveIn[i], sizeof(short), 1, fp);

		waveInStop(hWaveIn);		// ���� ����

		/* HANDLE �� buffer , ���� �ʱ�ȭ �ߴ� �������� ��ȯ */
		waveInReset(hWaveIn);		
		result = waveInUnprepareHeader(hWaveIn, &WaveInHdr, sizeof(WAVEHDR));
		result = waveInClose(hWaveIn);		 

		fclose(fp);
		fclose(bin);

		count_record = 0;
		record_value = 0;
		ttt = 0;
		remove("c:\\Project\\system\\test.bin");
		IMMEDIATELY_PLAY(file_name_wav);		// �ٷ� ��� �Լ� ȣ��
		system("cls");
		PLAY_LP(0);
	}
}

/* ��� ��� */
void IMMEDIATELY_PLAY(char record_file_name[23])		// ���� ���� �̸��� �Ű������� �޾ƿ�
{
	char file_name_play[50];
	char file_name_stop[50];
	char file_name_pause[50];
	char file_name_resume[50];
	char file_name_close[50];
	int record_key, record_true, record_value;

	record_value = 0;
	record_true = 1;

	/* ���ڿ� ���� */
	sprintf(file_name_play, "%s%s", MUSIC_PLAY ,record_file_name);
	sprintf(file_name_stop, "%s%s", MUSIC_STOP ,record_file_name);
	sprintf(file_name_pause, "%s%s", MUSIC_PAUSE, record_file_name);
	sprintf(file_name_resume, "%s%s", MUSIC_RESUME, record_file_name);
	sprintf(file_name_close, "%s%s", MUSIC_CLOSE, record_file_name);

	system("cls");
	GOTOXY(1,3);
	LIGHT_RED 
	printf("    ������ ������ ������ ������ ������  ���� ������ ����  �� ��������\n");
	printf("    ������ ������ ��     ��  �� ������ �� ��   ��   �������� ��  ����\n");
	printf("    ������ ������ ������ ������ ������  ���� ������ ��  ���� ��������\n");
	GOTOXY(1,8);
	LIGHT_WHITE printf("=========================================================================================================================================================================\n\n\n\n");
	GOTOXY(140,41);
	printf("- product by �� 1 �� 2");
	SELECT_RECORD_1(0);
	
	while(record_true)
	{
		record_key = getch();
		switch(record_key)
		{
		case 72 :			// UP
			if(record_value == 0 || record_value == 1)
			{
				record_value--;
				if(record_value < 0)
					record_value = 1;
				SELECT_RECORD_1(record_value);		// ���ǿ� �´� �Լ� ȣ��
			}
			else if(record_value == 3 || record_value == 4 || record_value == 5)
			{
				record_value--;
				if(record_value < 3)
					record_value = 5;
				SELECT_RECORD_3(record_value);		// ���ǿ� �´� �Լ� ȣ��
			}
			break;
		case 80 :			// DOWN
			if(record_value == 0 || record_value == 1)
			{
				record_value++;
				if(record_value > 1)
					record_value = 0;
				SELECT_RECORD_1(record_value);		// ���ǿ� �´� �Լ� ȣ��
			}
			else if(record_value == 3 || record_value == 4 || record_value == 5)
			{
				record_value++;
				if(record_value > 5)
					record_value = 3;
				SELECT_RECORD_3(record_value);		// ���ǿ� �´� �Լ� ȣ��
			}
			break;
		case 13 :
			if(record_value == 0)				// ENTER
			{
				mciSendString(file_name_play, NULL, 0, NULL);		// ���� ���� ���
				record_value = 2;
				SELECT_RECORD_2();
			}

			else if(record_value == 1)			// �Լ� Ż��
				record_true--;

			else if(record_value == 3)	
			{
				mciSendString(file_name_resume, NULL, 0, NULL);		// ���� ���� �Ͻ� ����
				SELECT_RECORD_2();
			}
			else if(record_value == 4)
			{
				mciSendString(file_name_stop, NULL, 0, NULL);		// ���� ���� ����
				mciSendString(file_name_play, NULL, 0, NULL);		// ���� ���� ����  --> ó������ �ٽ� ���
				SELECT_RECORD_2();
			}
			else if(record_value == 5)
			{
				mciSendString(file_name_stop, NULL, 0, NULL);		
				mciSendString(file_name_close, NULL, 0, NULL);
				record_true--;
			}
			break;
		case 0x1B :		//ESC
			if(record_value != 0 && record_value != 1)
			{
				mciSendString(file_name_pause, NULL, 0, NULL);
				record_value = 3;
				SELECT_RECORD_3(record_value);
			}
			break;
		}
	}
}

/* ��� ��� ���� �������̽� 1 */
void SELECT_RECORD_1(int sel_rec_value_1)
{
	int sel_rec_1;
	int sel_rec_x = 64;
	int sel_rec_y[] = {19,20,21,22,23, 26,27,28,29};
	char print_m[9][120] = {"      �� ������    ����         ��     ",
						    "�������� ������    ����   ��������",
						    "�������� ������   ������      ����",
							"         ������    ����         ��       ",
                            "                   ����        ",

						    " ������  ���� ������          ��       ��    ",
						    "     �������� ��  ��   ������ �� ��������  ",
						    "     �������� ������   ������ ��     ����   ",
						    "         ����   ����   ������ ��       ��     "};
	char *print_n[] = { print_m, print_m + 1, print_m + 2, print_m + 3, print_m + 4, print_m + 5, print_m + 6, print_m + 7, print_m + 8 };

	if(sel_rec_value_1 == 0)
	{
		for(sel_rec_1 = 0; sel_rec_1 < 5; sel_rec_1++)
		{
			GOTOXY(sel_rec_x+3,sel_rec_y[sel_rec_1]);
			LIGHT_YELLOW printf("%s", print_n[sel_rec_1]);
		}
		for(sel_rec_1 = 5; sel_rec_1 <9; sel_rec_1++)
		{
			GOTOXY(sel_rec_x,sel_rec_y[sel_rec_1]);
			LIGHT_WHITE printf("%s", print_n[sel_rec_1]);
		}
		GOTOXY(sel_rec_x-9,sel_rec_y[2]);LIGHT_YELLOW printf("������");
		GOTOXY(sel_rec_x-9,sel_rec_y[7]);LIGHT_YELLOW printf("      ");
	}
	if(sel_rec_value_1 == 1)
	{
		for(sel_rec_1 = 0; sel_rec_1 < 5; sel_rec_1++)
		{
			GOTOXY(sel_rec_x+4,sel_rec_y[sel_rec_1]);
			LIGHT_WHITE printf("%s", print_n[sel_rec_1]);
		}
		for(sel_rec_1 = 5; sel_rec_1 <9; sel_rec_1++)
		{
			GOTOXY(sel_rec_x-1,sel_rec_y[sel_rec_1]);
			LIGHT_YELLOW printf("%s", print_n[sel_rec_1]);
		}
		GOTOXY(sel_rec_x-9,sel_rec_y[2]);LIGHT_YELLOW printf("      ");
		GOTOXY(sel_rec_x-9,sel_rec_y[7]);LIGHT_YELLOW printf("������");
	}

}
/* ��� ��� ���� �������̽� 2 */
void SELECT_RECORD_2()
{
	system("cls");
	GOTOXY(1,3);
	LIGHT_RED 
	printf("    ������ ������ ������ ������ ������  ���� ������ ����  �� ��������\n");
	printf("    ������ ������ ��     ��  �� ������ �� ��   ��   �������� ��  ����\n");
	printf("    ������ ������ ������ ������ ������  ���� ������ ��  ���� ��������\n");
	GOTOXY(1,8);
	LIGHT_WHITE printf("=========================================================================================================================================================================\n\n\n\n");
	GOTOXY(140,41);
	printf("- product by �� 1 �� 2");

	GREEN GOTOXY(51,21); printf("��   ��������     ����������      �� ������ ����  �� ����������   ��  \n");
	GREEN GOTOXY(51,22); printf("��   ��������     ������������  ����   ��   �������� ��  ������   ��  \n");
	GREEN GOTOXY(51,23); printf("��   ��    ��     ��    ��  ������     ��   ��  ���� ��  ��  ��   ��  \n");
	GREEN GOTOXY(51,24); printf("��   ��    ������ ��    ��    ��     ������ ��  ���� ����������   ��  \n");
	LIGHT_AQUA GOTOXY(78,27); printf(" E S C ������");
}
/* ��� ��� ���� �������̽� 3 */
void SELECT_RECORD_3(int sel_rec_value_3)
{
	int sel_rec_3;
	int sel_rec_x = 58;
	int sel_rec_y[] = {16,17,18,19,20, 23,24,25,26,27,  30,31,32,33,34};
	
	char print_m[15][120] = {"������  ���� ������    �������� ����   ��  ����",
						     "    �������� ��  ��    �������� ���� ����������",
						     "    �������� ������    ��    �� ���� ��  ������",
						 	 "        ����   ����                       ����",
							 "                                         ���� ",
						 	 "       ��         ��   �������� ����   ��  ����",
							 "������ ��  ������ ��   �������� ���� ����������",
							 "��     ��  ��  �� ��   ��    �� ���� ��  ������",
							 "������ ��         ��                      ����",
							 "                                         ����",
							 "               ��          ��       ��",
						     " ������  ��������   ������ �� ��������",
						     "     ��  ��������   ������ ��     ����",
							 "     ��        ��   ������ ��       ��",
							 "��������   ������"};

	char *print_n[] = { print_m, print_m + 1, print_m + 2, print_m + 3, print_m + 4, print_m + 5, print_m + 6, print_m + 7, print_m + 8, print_m + 9, print_m + 10, print_m + 11, print_m + 12, print_m + 13, print_m + 14 };

	system("cls");
	GOTOXY(1,3);
	LIGHT_RED 
	printf("    ������ ������ ������ ������ ������  ���� ������ ����  �� ��������\n");
	printf("    ������ ������ ��     ��  �� ������ �� ��   ��   �������� ��  ����\n");
	printf("    ������ ������ ������ ������ ������  ���� ������ ��  ���� ��������\n");
	GOTOXY(1,8);
	LIGHT_WHITE printf("=========================================================================================================================================================================\n\n\n\n");
	GOTOXY(140,41);
	printf("- product by �� 1 �� 2");

	if(sel_rec_value_3 == 3)
	{
		for(sel_rec_3 = 0; sel_rec_3 < 5; sel_rec_3++)
		{
			GOTOXY(sel_rec_x+2 ,sel_rec_y[sel_rec_3]);
			LIGHT_YELLOW printf("%s", print_n[sel_rec_3]);
		}
		for(sel_rec_3 = 5; sel_rec_3 < 10; sel_rec_3++)
		{
			GOTOXY(sel_rec_x+3 ,sel_rec_y[sel_rec_3]);
			LIGHT_WHITE printf("%s", print_n[sel_rec_3]);
		}
		for(sel_rec_3 = 10; sel_rec_3 < 15; sel_rec_3++)
		{
			GOTOXY(sel_rec_x+7 ,sel_rec_y[sel_rec_3]);
			LIGHT_WHITE printf("%s", print_n[sel_rec_3]);
		}
		GOTOXY(sel_rec_x-9,sel_rec_y[2]);LIGHT_YELLOW printf("������");
		GOTOXY(sel_rec_x-9,sel_rec_y[7]);LIGHT_YELLOW printf("      ");
		GOTOXY(sel_rec_x-9,sel_rec_y[12]);LIGHT_YELLOW printf("      ");
	}
	if(sel_rec_value_3 == 4)
	{
		for(sel_rec_3 = 0; sel_rec_3 < 5; sel_rec_3++)
		{
			GOTOXY(sel_rec_x+3 ,sel_rec_y[sel_rec_3]);
			LIGHT_WHITE printf("%s", print_n[sel_rec_3]);
		}
		for(sel_rec_3 = 5; sel_rec_3 < 10; sel_rec_3++)
		{
			GOTOXY(sel_rec_x+2 ,sel_rec_y[sel_rec_3]);
			LIGHT_YELLOW printf("%s", print_n[sel_rec_3]);
		}
		for(sel_rec_3 = 10; sel_rec_3 < 15; sel_rec_3++)
		{
			GOTOXY(sel_rec_x+7 ,sel_rec_y[sel_rec_3]);
			LIGHT_WHITE printf("%s", print_n[sel_rec_3]);
		}
		GOTOXY(sel_rec_x-9,sel_rec_y[2]);LIGHT_YELLOW printf("      ");
		GOTOXY(sel_rec_x-9,sel_rec_y[7]);LIGHT_YELLOW printf("������");
		GOTOXY(sel_rec_x-9,sel_rec_y[12]);LIGHT_YELLOW printf("      ");
	}
	if(sel_rec_value_3 == 5)
	{
		for(sel_rec_3 = 0; sel_rec_3 < 5; sel_rec_3++)
		{
			GOTOXY(sel_rec_x+3 ,sel_rec_y[sel_rec_3]);
			LIGHT_WHITE printf("%s", print_n[sel_rec_3]);
		}
		for(sel_rec_3 = 5; sel_rec_3 < 10; sel_rec_3++)
		{
			GOTOXY(sel_rec_x+3 ,sel_rec_y[sel_rec_3]);
			LIGHT_WHITE printf("%s", print_n[sel_rec_3]);
		}
		for(sel_rec_3 = 10; sel_rec_3 < 15; sel_rec_3++)
		{
			GOTOXY(sel_rec_x+6 ,sel_rec_y[sel_rec_3]);
			LIGHT_YELLOW printf("%s", print_n[sel_rec_3]);
		}
		GOTOXY(sel_rec_x-9,sel_rec_y[2]);LIGHT_YELLOW printf("      ");
		GOTOXY(sel_rec_x-9,sel_rec_y[7]);LIGHT_YELLOW printf("      ");
		GOTOXY(sel_rec_x-9,sel_rec_y[12]);LIGHT_YELLOW printf("������");
	}
}

/* ������Ʈ ���� ���� �Լ� */
void Project_select(int pro_sel_num){
   char project[5][50]={"project1\\", "project2\\", "project3\\", "project4\\", "project5\\"}; // ��� ����
   //������Ʈ �������� ���ϸ� ����
   char Project1[][100]={"a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7", "a8", "a9", "a10", "a11", "a12", "a13", "a14", "a15", "a16", "a17", "a18", "a19", "a20", "a21", "a22", "a23", "a24", "a25", "a26", "a27", "a28", "a29"};
   char Project2[][100]={"a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7", "a8", "a9", "a10", "a11", "a12", "a13", "a14", "a15", "a16", "a17", "a18", "a19", "a20", "a21", "a22", "a23", "a24", "a25", "a26", "a27", "a28", "a29"};
   char Project3[][100]={"a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7", "a8", "a9", "a10", "a11", "a12", "a13", "a14", "a15", "a16", "a17", "a18", "a19", "a20", "a21", "a22", "a23", "a24", "a25", "a26", "a27", "a28", "a29"};
   char Project4[][100]={"a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7", "a8", "a9", "a10", "a11", "a12", "a13", "a14", "a15", "a16", "a17", "a18", "a19", "a20", "a21", "a22", "a23", "a24", "a25", "a26", "a27", "a28", "a29"};
   char Project5[][100]={"a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7", "a8", "a9", "a10", "a11", "a12", "a13", "a14", "a15", "a16", "a17", "a18", "a19", "a20", "a21", "a22", "a23", "a24", "a25", "a26", "a27", "a28", "a29"};
   
   int i=0;
   int pro1_num=0, pro2_num=0, pro3_num=0, pro4_num=0, pro5_num=0;

   system("cls");

   if(pro_sel_num==1){ // ����ڰ� 1��������Ʈ�� ������ ���
      pro1_num=sizeof(Project1)/100; // ������Ʈ ���� ������ ������ ����
      for(i=0; i<pro1_num; i++){
      //1�� ������Ʈ�� �ش��ϴ� �Ķ���� ����
      sprintf(Sel_project_start[i], "%s%s%s%s%s", MUSIC_PLAY, LINK, project[0],  Project1[i], FILE_EXTENSION); // "Play+ �����̸�+Ȯ����" ���ڿ� ����
      sprintf(Sel_project_stop[i], "%s%s%s%s%s", MUSIC_STOP, LINK, project[0],  Project1[i], FILE_EXTENSION); // "Stop+�����̸�+Ȯ����" ���ڿ� ����
      }
      return;
   }

   if(pro_sel_num==2){ // ����ڰ� 2��������Ʈ�� ������ ���
      pro2_num=sizeof(Project2)/100;
      for(i=0; i<pro2_num; i++){
      //2�� ������Ʈ�� �ش��ϴ� �Ķ���� ����
      sprintf(Sel_project_start[i], "%s%s%s%s%s", MUSIC_PLAY, LINK, project[1],  Project2[i], FILE_EXTENSION); // "Play+ �����̸�+Ȯ����" ���ڿ� ����
      sprintf(Sel_project_stop[i], "%s%s%s%s%s", MUSIC_STOP, LINK, project[1],  Project2[i], FILE_EXTENSION); // "Stop+�����̸�+Ȯ����" ���ڿ� ����
      }
      return;
   }

   if(pro_sel_num==3){ // ����ڰ� 3��������Ʈ�� ������ ���
      pro3_num=sizeof(Project3)/100;
      for(i=0; i<pro3_num; i++){
      //3�� ������Ʈ�� �ش��ϴ� �Ķ���� ����
      sprintf(Sel_project_start[i], "%s%s%s%s%s", MUSIC_PLAY, LINK, project[2],  Project3[i], FILE_EXTENSION); // "Play+ �����̸�+Ȯ����" ���ڿ� ����
      sprintf(Sel_project_stop[i], "%s%s%s%s%s", MUSIC_STOP, LINK, project[2],  Project3[i], FILE_EXTENSION); // "Stop+�����̸�+Ȯ����" ���ڿ� ����
      }
      return;
   }

   if(pro_sel_num==4){ // ����ڰ� 4��������Ʈ�� ������ ���
      pro4_num=sizeof(Project4)/100;
      for(i=0; i<pro4_num; i++){
      //4�� ������Ʈ�� �ش��ϴ� �Ķ���� ����
      sprintf(Sel_project_start[i], "%s%s%s%s%s", MUSIC_PLAY, LINK, project[3],  Project4[i], FILE_EXTENSION); // "Play+ �����̸�+Ȯ����" ���ڿ� ����
      sprintf(Sel_project_stop[i], "%s%s%s%s%s", MUSIC_STOP, LINK, project[3],  Project4[i], FILE_EXTENSION); // "Stop+�����̸�+Ȯ����" ���ڿ� ����
      }
      return;
   }

   if(pro_sel_num==5){ // ����ڰ� 5��������Ʈ�� ������ ���
      pro5_num=sizeof(Project5)/100;
      for(i=0; i<pro5_num; i++){
      //5�� ������Ʈ�� �ش��ϴ� �Ķ���� ����
      sprintf(Sel_project_start[i], "%s%s%s%s%s", MUSIC_PLAY, LINK, project[4],  Project5[i], FILE_EXTENSION); // "Play+ �����̸�+Ȯ����" ���ڿ� ����
      sprintf(Sel_project_stop[i], "%s%s%s%s%s", MUSIC_STOP, LINK, project[4],  Project5[i], FILE_EXTENSION); // "Stop+�����̸�+Ȯ����" ���ڿ� ����
      }
      return;
   }
}