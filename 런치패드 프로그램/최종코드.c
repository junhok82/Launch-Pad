#include <stdio.h>
#include <Windows.h>
#include <conio.h>
#include <process.h>
#include <stdlib.h>
#include <time.h>
#include <MMSystem.h>
#include "wave.h"

#pragma comment(lib,"winmm.lib")

/* 녹음 관련 매크로 */
#define bps 32
#define channel 2
#define duration 180
#define sampleRate 44100

/* 색깔 관련 매크로 */
#define HD GetStdHandle(STD_OUTPUT_HANDLE)
#define GREEN SetConsoleTextAttribute((HD), 10);
#define VOILET SetConsoleTextAttribute((HD), 13);
#define LIGHT_BLUE SetConsoleTextAttribute((HD), FOREGROUND_BLUE | FOREGROUND_INTENSITY);
#define LIGHT_AQUA SetConsoleTextAttribute((HD), FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
#define LIGHT_RED SetConsoleTextAttribute((HD), FOREGROUND_RED | FOREGROUND_INTENSITY);
#define LIGHT_PURPLE SetConsoleTextAttribute((HD), FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
#define LIGHT_YELLOW SetConsoleTextAttribute((HD), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
#define LIGHT_WHITE SetConsoleTextAttribute((HD), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);

/* 프로젝트선택 관련 매크로 */
#define MUSIC_PLAY "play "
#define MUSIC_STOP "stop "
#define MUSIC_PAUSE "pause "
#define MUSIC_RESUME "resume "
#define MUSIC_CLOSE "close "
#define LINK "c:\\Project\\"
#define FILE_EXTENSION ".wav"
#define RECORD_LINK "c:\\Project\\record\\"

/* 기타 매크로 */
#define DELAY 8
#define BUTTON_X 39
#define BUTTON_Y 13

CRITICAL_SECTION m_csGDILock;		// 쓰레드 동기화 관련

/* 인터페이스 관련 함수 */
void INTERFACE01();
void INTERFACE02();

/* 메뉴 관련 함수 */
void MENU();
void SELECT_MU(int sel_mu_link);

/* 런치패드 화면 및 키 제어 관련 함수 */
void PLAY_LP(int pl_link);
void SELECT_LP_1(int sel_lp_1_link_1, int sel_lp_1_link_2);

/* 녹음 파일 불러오기 관련 함수 */
void RECORD_LOAD();
void RECORD_UNLOAD();
void record_search();
WIN32_FIND_DATA FileData[100];

/* 듀토리얼 관련 함수 */
void TUTO_1();
void TUTO_2();

/* 녹음 관련 함수 */
void START_RECORD(int record__link);
void IMMEDIATELY_PLAY(char *record_file_name);
void SELECT_RECORD_1(int sel_rec__value_1);
void SELECT_RECORD_2();
void SELECT_RECORD_3(int sel_rec__value_3);

/* 프로젝트선택 관련 함수 */
void PROJECT_1(int pr_1_link);
void SELECT_LP_2(int sel_lp_2_link);
void Project_select(int pro_sel_num);

/* 기타 함수 */
void REMOVECURSOR();
void GOTOXY(int x, int y);
void BUTTON(int bu_x, int bu_y);

/* 녹음 파일 내용을 저장하는 구조체 */
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

unsigned short ttt = 0;					// 녹음 관련해서 사용하는 변수
short waveIn[sampleRate * duration * channel * (int)(bps / 8)];		// 녹음 관련해서 사용하는 변수
int count_record = 0, record_value = 0;			// 녹음 관련해서 사용하는 변수

/* 쓰레드에서 사용 할 변수 */
int key_state[30] = { 0 }; 
int last_state[30] = { 0 };
int count[30] = { 0 };

/* 프로젝트선택 관련 변수 */
char Sel_project_start[30][100]={0};
char Sel_project_stop[30][100]={0};

/* 쓰레드 각 키마다 적용 (키 입력시 사운드 재생 입력 취소시 사운드 정지) */
void __stdcall T0(void *arg)			// 'Q'
{ 
	while (1)
	{
		key_state[0] = GetAsyncKeyState('Q');		// 키 입력 상태를 감지하는 함수
		if (key_state[0] & 0x8000)					// 키가 입력 되었을 경우
		{
			EnterCriticalSection(&m_csGDILock);		// 쓰레드 동기화 관련
			LIGHT_BLUE BUTTON(BUTTON_X,BUTTON_Y);	// 색 출력
			LeaveCriticalSection(&m_csGDILock);		// 쓰레드 동기화 관련
			Sleep(DELAY);
			mciSendString(Sel_project_start[0], NULL, 0, NULL);		// 키에 맞는 사운드 출력
																	// Sel_project_start -> 문자열 조합 (아래 참고)
			last_state[0] = 1;
		}
		else
		{
			if (last_state[0] == 1)									// 현재 키 입력이 되지 않고 있는 상태 이며, 이전에 키를 누르고 있었을 경우
			{
				EnterCriticalSection(&m_csGDILock);
				LIGHT_WHITE BUTTON(BUTTON_X,BUTTON_Y);
				LeaveCriticalSection(&m_csGDILock);
				mciSendString(Sel_project_stop[0], NULL, 0, NULL);	// 사운드 정지
				last_state[0] = 0;
				count[0] = 0;
				return;												// 쓰레드 종료
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

/* 메인 함수 */
int main(void)
{
	InitializeCriticalSection(&m_csGDILock);		// 쓰레드 동기화 관련
	
	system("title 런 치 패 드 프 로 그 램");		// 타이틀 정의
	system("mode con: cols=165 lines=35");			// 콘솔창 크기 제어
	REMOVECURSOR();									// 커서 제거

	Project_select(1);								// 프로그램 시작 시 적용되는 프로젝트 파일(음원 파일)
	mciSendString("play c:\\Project\\system\\bgm.wav", NULL, 0, NULL);		// BGM 재생

	INTERFACE01(); LIGHT_RED INTERFACE01(); GREEN INTERFACE01(); LIGHT_PURPLE INTERFACE01(); LIGHT_AQUA INTERFACE01();
	INTERFACE02();
	getch();

	system("cls");
	system("mode con: cols=170 lines=43");

	MENU();

	return 0;
} 

/* 각종 키 입력 시 적용되는 기능을 가진 함수 */
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
			if(count_record == 1)		// 녹음 중지 시
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

/* 인터페이스 1 */
void INTERFACE01(void)
{
	GOTOXY(1,6);
	printf("=====================================================================================================================================================================\n");
	printf("                                                           \n");
	printf("  ■■■■■■■■■           ■■■                                  ■■■                                  ■■■  ■■■   ■■■■■■■■■■■■■■■■■\n");
	printf("  ■              ■           ■  ■                                  ■  ■                                  ■  ■  ■  ■   ■                              ■\n");
	printf("  ■■■■■■    ■           ■  ■              ■■■■            ■  ■   ■■■■■■■■■■■■■■■ ■  ■  ■  ■   ■    ■■■■■■■■■■■■■■\n");
	printf("            ■    ■           ■  ■              ■    ■            ■  ■   ■                          ■ ■  ■  ■  ■   ■    ■\n");
	printf("            ■    ■           ■  ■      ■■■■■    ■■■■■■  ■  ■   ■■■    ■■■■■    ■■■ ■  ■  ■  ■   ■    ■\n");
	printf("  ■■■■■■    ■ ■■■■■■  ■      ■                      ■  ■  ■       ■    ■      ■    ■     ■  ■■■  ■   ■    ■■■■■■■■■■■■■■\n");
	printf("  ■	          ■ ■            ■      ■■■■■      ■■■■■  ■  ■   ■■■    ■■■■■    ■■■ ■          ■   ■                              ■\n");
	printf("  ■  ■■■■■■■ ■■■■■■  ■              ■  ■  ■          ■  ■   ■                          ■ ■  ■■■  ■   ■■■■■■■■■■■■■■■■■\n");
	printf("  ■  ■                       ■  ■            ■  ■  ■  ■        ■  ■   ■■■■■■■■■■■■■■■ ■  ■  ■  ■\n");
	printf("  ■  ■■■■■■■           ■  ■          ■  ■      ■  ■      ■  ■                                  ■  ■  ■  ■   ■■■■■■■■■■■■■■■■■\n");
	printf("  ■              ■           ■  ■        ■  ■          ■  ■    ■  ■                                  ■  ■  ■  ■   ■                              ■\n");
	printf("  ■■■■■■■■■           ■  ■        ■■              ■■    ■  ■                                  ■  ■  ■  ■   ■■■■■■■■■■■■■■■■■\n");
	printf("                               ■■■                                  ■  ■                                  ■■■  ■■■\n");	
	printf("            ■■■                                                     ■  ■\n");	
	printf("            ■  ■                                                     ■■■\n");	
	printf("            ■  ■■■■■■■■■\n");	                     
	printf("            ■                  ■\n");	
	printf("            ■■■■■■■■■■■\n");	           
	printf("\n====================================================================================================================================================================\n");
	printf("                                                                         L o a d i n g . . . ");
	Sleep(700);
	system("cls");
}
/* 인터페이스 2 */
void INTERFACE02(void)
{	
	GOTOXY(1,7);
	LIGHT_WHITE printf("          ■■■■■   ■       ■■      ■  ■■■■■■ ■  ■   ■■■■■■\n");
	LIGHT_WHITE printf("                  ■   ■   ■■■■■■  ■    ■    ■   ■  ■   ■\n");
	LIGHT_WHITE printf("          ■■■■■ ■■       ■■      ■    ■    ■   ■■■   ■\n");
	LIGHT_WHITE printf("          ■           ■      ■  ■     ■  ■■■■■■ ■  ■   ■■■■■■\n");
	LIGHT_WHITE printf("          ■■■■■   ■     ■    ■    ■               ■  ■\n");
	LIGHT_WHITE printf("                ■                        ■                       ■■■■■■■\n");
	LIGHT_WHITE printf("                ■■■■■\n");
	LIGHT_WHITE printf("                                                                                                                                                                    \n");
	printf("                                                                                                                                                                    \n");
	printf("                                                                                                                                                                    \n");
	LIGHT_YELLOW printf("                                                                                                                    ■■    ■■     ■■■    ■■■■\n");
	LIGHT_YELLOW printf("                                     ■■■  ■■■              ■                                                ■  ■      ■    ■  ■          ■\n");
	LIGHT_YELLOW printf("                                     ■  ■  ■  ■              ■                    ■         ■    ■  ■      ■■       ■    ■■■    ■■■■    \n");
	LIGHT_YELLOW printf("                                     ■■■  ■■■  ■■■  ■■■  ■  ■   ■■  ■■■■      ■    ■■■   ■■■■■    ■   ■■■■   ■\n");
	LIGHT_YELLOW printf("                                     ■      ■■    ■  ■  ■  ■  ■  ■  ■        ■         ■■    ■       ■  ■      ■      ■      ■\n");
	LIGHT_YELLOW printf("                                     ■      ■  ■  ■■■  ■■■   ■■    ■■     ■■       ■■    ■       ■  ■    ■■■    ■      ■■■■\n");
	printf("\n\n\n\n\n\n");
	LIGHT_WHITE printf("                                                                  Press any key when you are ready!!");
}

/* 메뉴 함수 */
void MENU()
{
	int mu_i = 0, mu_key = NULL, mu_true = 1;

	mciSendString("play c:\\Project\\system\\bgm.wav", NULL, 0, NULL);
	LIGHT_WHITE
	printf("                    ___________________________________________________________________________________________________________________________________ \n");
	printf("                   /ㆍㆍㆍㆍㆍㆍㆍㆍㆍㆍㆍㆍㆍㆍㆍㆍㆍㆍㆍㆍㆍㆍㆍㆍㆍㆍㆍㆍㆍㆍㆍㆍㆍㆍㆍㆍㆍㆍㆍㆍㆍㆍㆍㆍㆍㆍㆍㆍㆍㆍㆍㆍㆍㆍㆍㆍㆍㆍㆍㆍㆍㆍㆍㆍ  /│\n");
	printf("                  ┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓:│\n");
    printf("                  ┃                               __                                                                                               ┃:│\n");                                   
    printf("                  ┃               __________     /:/│          ____         __                        __    __      ______________                ┃:│\n");                      
    printf("                  ┃              /ㆍㆍㆍㆍ /│   ■:│    _____/ㆍ /____    /:/│    ______________   /:/│ /:/│   /ㆍㆍㆍㆍㆍㆍ /│              ┃:│\n");
	printf("                  ┃              ■■■■■:│   ■:│   /ㆍㆍ■■ㆍㆍ /│  ■:│   /ㆍㆍㆍㆍㆍㆍ /│ ■:│ ■:│   ■■■■■■■/                ┃:│\n");
	printf("                  ┃               _______■:│   ■:│   ■■■■■■■/    ■:│   ■■■■■■■/   ■:│_■:│   ■:|                           ┃:│\n");
    printf("                  ┃              /ㆍㆍㆍ ■:│   ■:│        ■■:│       ■:│     ■:│    ■:│  ■: ㆍ■:│   ■:|                           ┃:│\n");
	printf("                  ┃              ■■■■■/   ■■:│       ■:│■:│     ■:│    _■:│___ ■:│  ■■■■:│   ■:|___________                ┃:│\n");
    printf("                  ┃              ■:│           ■:│      ■:│  ■:│    ■:│   /:■ /ㆍㆍ/■:/│ ■:│ ■:│   ■/ㆍㆍㆍㆍㆍ /│              ┃:│\n");
    printf("                  ┃              ■:│______     ■:│     ■:│    ■:│   ■:│   ■■■■■■■/   ■:│ ■:│   ■■■■■■■/                ┃:│\n");                               
	printf("                  ┃              ■/ㆍㆍㆍ /│   ■:│    ■:│      ■:│  ■:│                     ■/   ■/                                    ┃:│\n");
	printf("                  ┃              ■■■■■/     ■/     ■_/         ■_/  ■/                                   ____________________             ┃:│\n");
    printf("                  ┃                    __                                                                        /ㆍㆍㆍㆍㆍㆍㆍㆍㆍ /│           ┃:│\n");
    printf("                  ┃                   /:/│                                                                      ■■■■■■■■■■/             ┃:│\n");              
	printf("                  ┃                   ■:│________                                                                                                ┃:│\n");
	printf("                  ┃                   ■/ㆍㆍㆍㆍ /│                                                                                              ┃:│\n");
	printf("                  ┃                   ■■■■■■/                                                                                                ┃:│\n");
	printf("                  ┃                                                                                                                                ┃:│\n");
	printf("                  ┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛ / \n\n");
	printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
	GOTOXY(1,39);
	printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");

	SELECT_MU(mu_i);		//	이미지 출력

	GOTOXY(140,41);
	LIGHT_WHITE printf("- product by 유 1 무 2");

		while (mu_true) 
		{
			mu_key = getch();
			switch (mu_key)
			{
			case 72:		// 방향키 위
				mu_i--;
				if (mu_i < 0)
					mu_i=2;
				SELECT_MU(mu_i);
				break;
			case 80:		// 방향키 아래
				mu_i++;
				if (mu_i > 2) 
					mu_i=0;
				SELECT_MU(mu_i);
				break;
			case 13:		// 엔터
				if (mu_i == 0)		// 런치패드 실행
				{
					mciSendString("stop c:\\Project\\system\\button.wav", NULL, 0, NULL);
					mciSendString("play c:\\Project\\system\\button.wav", NULL, 0, NULL);
					system("cls");
					PLAY_LP(0);
					mu_true--;
				}
				if (mu_i == 1)		// 녹음파일 불러오기
				{
					mciSendString("stop c:\\Project\\system\\button.wav", NULL, 0, NULL);
					mciSendString("play c:\\Project\\system\\button.wav", NULL, 0, NULL);
					system("cls");
					RECORD_LOAD();
					mu_true--;
				}
				if (mu_i == 2)		// 종료하기
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

/* 메뉴 선택 인터페이스 */
void SELECT_MU(int sel_mu_link)
{
	int g;
	int sel_mu__x = 40;
	int sel_mu__y[] = {25,26,27, 30,31,32, 35,36,37};

	char p[9][100] =  { ("                            ┏━━ ━┳━ ┏━┓ ┏━┓ ━┳━"),
						("                            ┗━┓   ┃   ┣━┫ ┣┳┛   ┃  "),
						("                            ━━┛   ┃   ┃  ┃ ┃┗━   ┃  "),
					    ("                    ┃    ┏━┓┏━┓ ━┓   ┏━━ ━┳━ ┃     ┏━━"),
						("                    ┃    ┃  ┃┣━┫┃ ┃   ┣━━   ┃   ┃     ┣━━"),
						("                    ┗━━┗━┛┃  ┃ ━┛   ┃     ━┻━ ┗━━ ┗━━"),
						("                             ┏━━  ━┓┏━  ━┳━  ━┳━"),
						("                             ┣━━    ┣┫      ┃      ┃  "),
						("                             ┗━━  ━┛┗━  ━┻━    ┃  "),
					};
	char *q[]= {p, p+1, p+2, p+3, p+4, p+5, p+6, p+7, p+8};

	if(sel_mu_link == 0)
	{
		for(g=0; g<9; g++)			// 전체 인터페이스 출력
		{
			GOTOXY(sel_mu__x,sel_mu__y[g]);
			LIGHT_WHITE printf("%s", q[g]);
		}
		for(g=0; g<3; g++)			// start만 노란색으로 출력
		{
			GOTOXY(sel_mu__x-1,sel_mu__y[g]);
			LIGHT_YELLOW printf("%s", q[g]);
		}
		GOTOXY(sel_mu__x+6,sel_mu__y[1]); printf("━━▶");
	}
	else if(sel_mu_link == 1)
	{
	for(g=0; g<9; g++)			// 전체 인터페이스 출력
		{
			GOTOXY(sel_mu__x,sel_mu__y[g]);
			LIGHT_WHITE printf("%s", q[g]);
		}
		for(g=3; g<6; g++)			// load file만 노란색으로 출력
		{
			GOTOXY(sel_mu__x-1,sel_mu__y[g]);
			LIGHT_YELLOW printf("%s", q[g]);
		}
		GOTOXY(sel_mu__x+6,sel_mu__y[4]); printf("━━▶");
	}
	else if(sel_mu_link == 2)
	{
		for(g=0; g<9; g++)			// 전체 인터페이스 출력
		{
			GOTOXY(sel_mu__x,sel_mu__y[g]);
			LIGHT_WHITE printf("%s", q[g]);
		}
		for(g=6; g<9; g++)			// exit만 노란색으로 출력
		{
			GOTOXY(sel_mu__x-1,sel_mu__y[g]);
			LIGHT_YELLOW printf("%s", q[g]);
		}
		GOTOXY(sel_mu__x+6,sel_mu__y[7]); printf("━━▶");
	}

}

/* 런치패드 메인 함수 */
void PLAY_LP(int pl_link)
{
	mciSendString("stop c:\\Project\\system\\bgm.wav", NULL, 0, NULL);
	if(pl_link == 0)		// 단순 조건 출력
	{
		LIGHT_WHITE
		printf("\n\n\n                                                                                                F1 녹음 시작                F3 튜토리얼                Esc 나가기\n");
		GOTOXY(1,3);
		LIGHT_RED 
		printf("    ┃    ┏━┓┃  ┃┏━┓┏━┃    ┏━┓┏━┓ ━┓\n");
		printf("    ┃    ┣━┫┃  ┃┃  ┃┃  ┣━┓┣━┛┣━┫┃ ┃\n");
		printf("    ┗━━┃  ┃┗━┛┃  ┃┗━┃  ┃┃    ┃  ┃ ━┛\n");
	}
	if(pl_link == 1 || pl_link == 2)		// 단순 조건 출력
	{
		LIGHT_WHITE
		printf("\n\n\n                                                                                                                                               Esc 튜토리얼 종료\n");
		GOTOXY(1,3);
		LIGHT_RED
		printf("    ━┳━                                  ┃\n");
		printf("      ┃   ┃  ┃  ┃   ━┓┃    ━┏━    ┃\n");
		printf("      ┃   ┃  ┃━╋━┃ ┃┣━┓┃┃  ┃  ┃\n");
		printf("      ┃   ┗━┛  ┗  ┗━ ┃    ┃  ━┻┓┃\n");
	}
	if(pl_link == 0 || pl_link == 1 || pl_link == 2)		// 단순 조건 출력
	{
		GOTOXY(139,10); LIGHT_WHITE printf("♬");
		GOTOXY(67,10);GREEN printf("♬");
		GOTOXY(1,8);
		LIGHT_WHITE printf("=========================================================================================================================================================================\n\n\n\n");
		printf("                                    ┌----------------------------------------------------------┐                   Ⅱ\n");
		printf("                                    ｜■■■■｜■■■■｜■■■■｜■■■■｜■■■■｜■■■■｜                   Ⅱ           프로젝트 파일 선택\n");
		printf("                                    ｜■■■■｜■■■■｜■■■■｜■■■■｜■■■■｜■■■■｜                   Ⅱ\n");
		printf("                                    ｜■■■■｜■■■■｜■■■■｜■■■■｜■■■■｜■■■■｜                   Ⅱ\n");
		printf("                                    ｜■■■■｜■■■■｜■■■■｜■■■■｜■■■■｜■■■■｜                   Ⅱ               Project 1\n");
		printf("                                    ｜----------------------------------------------------------｜                   Ⅱ\n");
		printf("                                    ｜■■■■｜■■■■｜■■■■｜■■■■｜■■■■｜■■■■｜                   Ⅱ\n");
		printf("                                    ｜■■■■｜■■■■｜■■■■｜■■■■｜■■■■｜■■■■｜                   Ⅱ               Project 2\n");
		printf("                                    ｜■■■■｜■■■■｜■■■■｜■■■■｜■■■■｜■■■■｜                   Ⅱ\n");
		printf("                                    ｜■■■■｜■■■■｜■■■■｜■■■■｜■■■■｜■■■■｜                   Ⅱ\n");
		printf("                                    ｜----------------------------------------------------------｜                   Ⅱ               Project 3\n");
		printf("                                    ｜■■■■｜■■■■｜■■■■｜■■■■｜■■■■｜■■■■｜                   Ⅱ\n");
		printf("                                    ｜■■■■｜■■■■｜■■■■｜■■■■｜■■■■｜■■■■｜                   Ⅱ\n");
		printf("                                    ｜■■■■｜■■■■｜■■■■｜■■■■｜■■■■｜■■■■｜                   Ⅱ               Project 4\n");
		printf("                                    ｜■■■■｜■■■■｜■■■■｜■■■■｜■■■■｜■■■■｜                   Ⅱ\n");
		printf("                                    ｜----------------------------------------------------------｜                   Ⅱ\n");
		printf("                                    ｜■■■■｜■■■■｜■■■■｜■■■■｜■■■■｜■■■■｜                   Ⅱ               Project 5\n");
		printf("                                    ｜■■■■｜■■■■｜■■■■｜■■■■｜■■■■｜■■■■｜                   Ⅱ\n");
		printf("                                    ｜■■■■｜■■■■｜■■■■｜■■■■｜■■■■｜■■■■｜                   Ⅱ\n");
		printf("                                    ｜■■■■｜■■■■｜■■■■｜■■■■｜■■■■｜■■■■｜                   Ⅱ\n");
		printf("                                    ｜----------------------------------------------------------｜                   Ⅱ\n");
		printf("                                    ｜■■■■｜■■■■｜■■■■｜■■■■｜■■■■｜■■■■｜                   Ⅱ\n");
		printf("                                    ｜■■■■｜■■■■｜■■■■｜■■■■｜■■■■｜■■■■｜                   Ⅱ\n");
		printf("                                    ｜■■■■｜■■■■｜■■■■｜■■■■｜■■■■｜■■■■｜                   Ⅱ\n");
		printf("                                    ｜■■■■｜■■■■｜■■■■｜■■■■｜■■■■｜■■■■｜                   Ⅱ\n");
		printf("                                    └----------------------------------------------------------┘                   Ⅱ\n");;
		GOTOXY(140,41);
		printf("- product by 유 1 무 2");
	}
	
	if(pl_link == 2)		// 튜토리얼의 연장선
		TUTO_2();

	if(pl_link == 0)		// 연주 및 인터페이스 출력
	{
		PROJECT_1(0);
		SELECT_LP_1(1, 0);
	}
}

/* 런치패드 작동 함수 */
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
		case 80:				//  아래
			mciSendString("stop c:\\Project\\system\\button2.wav", NULL, 0, NULL);
			mciSendString("play c:\\Project\\system\\button2.wav", NULL, 0, NULL);
			sel_lp_1_link_1++;
			if(sel_lp_1_link_1>5)
				sel_lp_1_link_1=1;
			SELECT_LP_2(sel_lp_1_link_1);
			break;
		case 72:				// 위
			mciSendString("stop c:\\Project\\system\\button2.wav", NULL, 0, NULL);
			mciSendString("play c:\\Project\\system\\button2.wav", NULL, 0, NULL);
			sel_lp_1_link_1--;
			if(sel_lp_1_link_1<1)
				sel_lp_1_link_1=5;
			SELECT_LP_2(sel_lp_1_link_1);
			break;
		case 13:				// Enter, project 선택 기능(project 1)
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
					PLAY_LP(2); // 튜토리얼부분
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
/* 프로젝트 인터페이스 출력 함수 */
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


	if(sel_lp_2_link == 0)		//		♬ 초록색으로 바꿈
	{
		GOTOXY(139,10); LIGHT_WHITE printf("♬");
		GOTOXY(67,10); GREEN printf("♬");

		for(g=0; g<5; g++)		//		전체 출력
		{
			GOTOXY(sel_lp_2__x,sel_lp_2__y[g]);
			LIGHT_WHITE printf("%s", q[g]);
		}
	}
	if(sel_lp_2_link == 1)		//		♬ 하얀색으로 바꿈
	{
		GOTOXY(139,10); GREEN printf("♬");
		GOTOXY(67,10); LIGHT_WHITE printf("♬");

		for(g=0; g<5; g++)		//		project 1을 노란색으로 출력
		{
			GOTOXY(sel_lp_2__x,sel_lp_2__y[g]);
			LIGHT_WHITE printf("%s", q[g]);
		}
		GOTOXY(sel_lp_2__x-1,sel_lp_2__y[0]); LIGHT_YELLOW printf(" %s", q[0]);
		GOTOXY(sel_lp_2__x,sel_lp_2__y[0]);  printf("  ☞");
	}
	if(sel_lp_2_link == 2)
	{
		GOTOXY(139,10); GREEN printf("♬");
		GOTOXY(67,10); LIGHT_WHITE printf("♬");

		for(g=0; g<5; g++)		//		project 2을 노란색으로 출력
		{
			GOTOXY(sel_lp_2__x,sel_lp_2__y[g]);
			LIGHT_WHITE printf("%s", q[g]);
		}
		GOTOXY(sel_lp_2__x-1,sel_lp_2__y[1]); LIGHT_YELLOW printf(" %s", q[1]);
		GOTOXY(sel_lp_2__x,sel_lp_2__y[1]); printf("  ☞");
	}
	if(sel_lp_2_link == 3)
	{
		GOTOXY(139,10); GREEN printf("♬");
		GOTOXY(67,10); LIGHT_WHITE printf("♬");

		for(g=0; g<5; g++)		//		project 3을 노란색으로 출력
		{
			GOTOXY(sel_lp_2__x,sel_lp_2__y[g]);
			LIGHT_WHITE printf("%s", q[g]);
		}
		GOTOXY(sel_lp_2__x-1,sel_lp_2__y[2]); LIGHT_YELLOW printf(" %s", q[2]);
		GOTOXY(sel_lp_2__x,sel_lp_2__y[2]); printf("  ☞");
	}
	if(sel_lp_2_link == 4)
	{
		GOTOXY(139,10); GREEN printf("♬");
		GOTOXY(67,10); LIGHT_WHITE printf("♬");

		for(g=0; g<5; g++)		//		project 4을 노란색으로 출력
		{
			GOTOXY(sel_lp_2__x,sel_lp_2__y[g]);
			LIGHT_WHITE printf("%s", q[g]);
		}
		GOTOXY(sel_lp_2__x-1,sel_lp_2__y[3]); LIGHT_YELLOW printf(" %s", q[3]);
		GOTOXY(sel_lp_2__x,sel_lp_2__y[3]); printf("  ☞");
	}
	if(sel_lp_2_link == 5)
	{
		GOTOXY(139,10); GREEN printf("♬");
		GOTOXY(67,10); LIGHT_WHITE printf("♬");

		for(g=0; g<5; g++)		//		project 5을 노란색으로 출력
		{
			GOTOXY(sel_lp_2__x,sel_lp_2__y[g]);
			LIGHT_WHITE printf("%s", q[g]);
		}
		GOTOXY(sel_lp_2__x-1,sel_lp_2__y[4]); LIGHT_YELLOW printf(" %s", q[4]);
		GOTOXY(sel_lp_2__x,sel_lp_2__y[4]); printf("  ☞");
	}
}

/* 녹음 파일 불러오기 */
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
 printf("\n\n                                                                 ↑↓ 녹음파일선택      ←→ 페이지선택      Enter 녹음파일재생      Space 재생정지      Esc 나가기\n");
 GOTOXY(66,6); printf(" F3 녹음파일 검색");
 GOTOXY(1,3);
 LIGHT_RED 
 printf("    ┃    ┏━┓┃  ┃┏━┓┏━┃    ┏━┓┏━┓ ━┓\n");
 printf("    ┃    ┣━┫┃  ┃┃  ┃┃  ┣━┓┣━┛┣━┫┃ ┃\n");
 printf("    ┗━━┃  ┃┗━┛┃  ┃┗━┃  ┃┃    ┃  ┃ ━┛\n");


GOTOXY(1,8);
LIGHT_WHITE printf("=========================================================================================================================================================================\n\n\n");



 mciSendString("stop c:\\Project\\system\\bgm.wav", NULL, 0, NULL);
 hSearch = FindFirstFile("c:\\Project\\record\\*.wav", &FileData[0]); // 해당경로에 파일이 있을경우 핸들값 반환, 아닐경우 INVALID_HANDLE_VALUE 반환
 if (hSearch == INVALID_HANDLE_VALUE){
	 LIGHT_WHITE printf("녹음파일이 없습니다\nESC키를 누르면 메인메뉴로 돌아갑니다.\n");
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
 // 파일 검색시작
 i=0;
 while (fFinished) //초기값 TRUE 
 {
	 //LIGHT_WHITE printf("%s\n", FileData[i].cFileName);
	 if (!FindNextFile(hSearch, &FileData[i+1])) // FindNextFile 이용해 파일을 검색함(파일이 있을경우 0이아닌값 반환, 없을경우 0반환)
		 // FindNextFile 함수가 파일의 끝을 찾아서 0을 반환할 경우 if문이 실행됨(0의 반대이기 때문에)
	 {
		 if (GetLastError() == ERROR_NO_MORE_FILES) // 에러가 더이상 파일이 없다라는 에러일 경우
		 {
			 record_count=i;
			 fFinished = FALSE; // FALSE로 만들어서 반복문을 탈출함
		 }
	 }
	 i++;
 }

 GOTOXY(70, y[cursor]); LIGHT_YELLOW printf("☞");

 while(1){
	 for(i=first_page; i<last_page; i++){
		 GOTOXY(80, 11); LIGHT_WHITE printf("페이지 %2d/10", (i/10)+1);
		 
		 if(i>record_count){
			 GOTOXY(x, y[j]); LIGHT_WHITE printf("녹음파일 %2d : 빈슬롯\t\t\t\t\t\t\t\t\t\n", i+1);
			 j++;
			 if(j==10) j=0;
			 continue;
		 }
		GOTOXY(x, y[j]); LIGHT_WHITE printf("녹음파일 %2d : %s\t\t\t\t\t\t\t\t\t\n", i+1, FileData[i].cFileName);
		j++;  
		if(j==10) j=0;
	 }
	 key=getch();
	 switch(key)
	 {
	 case 77:// 방향키 오른쪽
		 first_page+=10;
		 last_page+=10;
		 if(first_page==100){
			 first_page=0;
			 last_page=10;
		 }
		 break;
	 case 75:// 방향키 왼쪽
		 first_page-=10;
		 last_page-=10;
		 if(first_page==-10){
			 first_page=90;
			 last_page=100;
		 }
		 break;
	 case 72:// 방향키 위
		 cursor--;
		 GOTOXY(70, y[cursor+1]); printf("   ");
		 GOTOXY(70, y[cursor]); LIGHT_YELLOW printf("☞");
		 if(cursor<0){
			 GOTOXY(70, y[0]); printf("    ");
			 GOTOXY(70, y[9]); LIGHT_YELLOW printf("☞");
			 cursor=9;
		 }
		 break;
	 case 80:// 방향키 아래
		 cursor++;
		 GOTOXY(70, y[cursor-1]); printf("   ");
		 GOTOXY(70, y[cursor]); LIGHT_YELLOW printf("☞");
		 if(cursor>9){
			GOTOXY(70, y[9]); printf("    ");
			GOTOXY(70, y[0]); LIGHT_YELLOW printf("☞");
			cursor=0;
		 }
		 break;
	 case 13: // 엔터
		 if(first_page+cursor>record_count){
			 GOTOXY(79, 35); printf("빈슬롯입니다");
			 break;
		 }
		 sprintf(file_name, "%s%s%s", MUSIC_PLAY, RECORD_LINK, FileData[first_page+cursor].cFileName);
		 GOTOXY(79, 35); printf("\t\t\t\t\t\t\t");
		 mciSendString(file_name, NULL, 0, NULL);
		 break;
	 case 32: // 스페이스바
		 if(first_page+cursor>record_count){
			 break;
		 }
		 sprintf(file_name, "%s%s%s", MUSIC_STOP, RECORD_LINK, FileData[first_page+cursor].cFileName);
		 mciSendString(file_name, NULL, 0, NULL);
		 break;
	 case 61: // F3 (검색)
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

/* 녹음 파일 찾기 */
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
	printf("    ┃    ┏━┓┃  ┃┏━┓┏━┃    ┏━┓┏━┓ ━┓\n");
	printf("    ┃    ┣━┫┃  ┃┃  ┃┃  ┣━┓┣━┛┣━┫┃ ┃\n");
	printf("    ┗━━┃  ┃┗━┛┃  ┃┗━┃  ┃┃    ┃  ┃ ━┛\n");
	
	GOTOXY(1,8);
	LIGHT_WHITE printf("=========================================================================================================================================================================\n\n\n");

	
	LIGHT_WHITE GOTOXY(70, 12); printf("파일이름을 입력해주세요 : ");
	LIGHT_YELLOW gets(search_name);
		
	sprintf(file_name, "%s%s",search_name, FILE_EXTENSION);
		
	for(i=0; i<100; i++){
		if(!strcmp(file_name, FileData[i].cFileName)){
			break;
		}
	}
	if(i==100){

		LIGHT_WHITE
		GOTOXY(38, 18); printf("┌---------------------------------------------------------------------------------------------┐\n");
		GOTOXY(38, 19); printf("│                                                                                             │\n");
		GOTOXY(38, 20); printf("│                                                                                             │\n");
		GOTOXY(38, 21); printf("│                                                                                             │\n");
		GOTOXY(38, 22); printf("│                                                                                             │\n");
		GOTOXY(38, 23); printf("│                                                                                             │\n");
		GOTOXY(38, 24); printf("│                                                                                             │\n");
		GOTOXY(38, 25); printf("│                                                                                             │\n");
		GOTOXY(38, 26); printf("│                                                                                             │\n");
		GOTOXY(38, 27); printf("└---------------------------------------------------------------------------------------------┘\n");

		LIGHT_AQUA GOTOXY(75, 21); printf("찾는 파일이 없습니다");
		LIGHT_AQUA GOTOXY(69, 24); printf("Enter을 누르면 메뉴로 돌아갑니다.");
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
		GOTOXY(38, 18); printf("┌---------------------------------------------------------------------------------------------┐\n");
		GOTOXY(38, 19); printf("│                                                                                             │\n");
		GOTOXY(38, 21); printf("│                                                                                             │\n");
		GOTOXY(38, 22); printf("└---------------------------------------------------------------------------------------------┘\n");

		LIGHT_YELLOW GOTOXY(70, 20); printf("☞");
		LIGHT_WHITE  GOTOXY(75, 20); printf("녹음파일 %2d : %s\t\t\t\t\t\t\t\t\t\n", i+1, FileData[i].cFileName);

		LIGHT_YELLOW GOTOXY(x, y[cursor]); printf("▶");
		LIGHT_WHITE  GOTOXY(80, 25); printf("듣기");
		LIGHT_WHITE  GOTOXY(80, 27); printf("나가기");
		

		while(1){
			key=getch();
			switch(key){
			case 72:// 방향키 위
				cursor--; 
				GOTOXY(x, y[cursor+1]); printf("  ");
				GOTOXY(x, y[cursor]); LIGHT_YELLOW printf("▶");
				if(cursor<0){
					GOTOXY(x, y[0]); printf("    ");
					GOTOXY(x, y[1]); LIGHT_YELLOW printf("▶");
					cursor=1;
				}
				break;
			case 80:// 방향키 아래
				 cursor++;
				 GOTOXY(x, y[cursor-1]); printf("   ");
				 GOTOXY(x, y[cursor]); LIGHT_YELLOW printf("▶");
				 if(cursor>1){
					 GOTOXY(x, y[1]); printf("    ");
					 GOTOXY(x, y[0]); LIGHT_YELLOW printf("▶");
					 cursor=0;
				 }
				 break;
			case 13: // 엔터
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
	GOTOXY(bu_x, bu_y); printf("■■■■\n");
	GOTOXY(bu_x, bu_y+1); printf("■■■■\n");
	GOTOXY(bu_x, bu_y+2); printf("■■■■\n");
	GOTOXY(bu_x, bu_y+3); printf("■■■■\n");
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

/* 튜토리얼 함수 1 */
void TUTO_1()
{
	int tu_1_key, tu_1_true;
	tu_1_key = 0;
	tu_1_true = 1;

	system("cls");
	PLAY_LP(1);

	LIGHT_WHITE GOTOXY(59,4); printf("┏"); printf("           이 문자가 초록색인 파트가       됩니다        "); printf("┓");
	GREEN GOTOXY(68,4); printf("♬"); LIGHT_YELLOW GOTOXY(98,4); printf("활성화"); 
	LIGHT_WHITE GOTOXY(59,5); printf("┃"); printf("                      하기위해              을 누르세요  "); printf("┃");
	LIGHT_YELLOW GOTOXY(63,5); printf(" Project File을 변경"); GOTOXY(91,5); printf(" 방향키 오른쪽");
	LIGHT_WHITE GOTOXY(59,6); printf("┗"); LIGHT_YELLOW printf("                    → → → → → → →                 "); LIGHT_WHITE printf("┛");
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
			LIGHT_WHITE GOTOXY(59,4); printf("┏"); printf("                        해서 프로젝트 파일을      및       "); printf("┓");
			LIGHT_YELLOW GOTOXY(72,4); printf("방향키를 조종"); GOTOXY(106,4); printf("선택");
			LIGHT_WHITE GOTOXY(59,5); printf("┃"); printf("                문자를         시킬수 있습니다             "); printf("┃");
			GREEN GOTOXY(74,5); printf("♬"); GOTOXY(85,5); LIGHT_YELLOW printf("활성화");
			LIGHT_WHITE GOTOXY(59,6); printf("┗"); printf("                                  해보세요                 "); printf("┛");
			LIGHT_YELLOW GOTOXY(79,6); printf("Project 1을 선택");
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

/* 튜토리얼 함수 2 */
void TUTO_2()
{
	int tu_2_value=1,tu_2_true=1;

	while(tu_2_true)		// 순차적으로 5번 진행 " tu_2_value++ "를 이용
	{
		if(tu_2_value == 1)
		{
			EnterCriticalSection(&m_csGDILock);
			LIGHT_WHITE GOTOXY(68,6); printf("┃ '          '을 길게 눌러보세요! ┃");
			LIGHT_YELLOW GOTOXY(73,6); printf("Numpad 0");
			LeaveCriticalSection(&m_csGDILock);
			PROJECT_1(tu_2_value);			// 조건에 맞는 사운드 출력
			tu_2_value++;
		}
		if(tu_2_value == 2)
		{
			Sleep(400);
			EnterCriticalSection(&m_csGDILock);
			LIGHT_WHITE GOTOXY(68,6); printf("┃ '       '을 길게 눌러보세요!   ┃");
			LIGHT_YELLOW GOTOXY(73,6); printf("Key Q");
			LeaveCriticalSection(&m_csGDILock);
			PROJECT_1(tu_2_value);
			tu_2_value++;
		}
		if(tu_2_value == 3)
		{
			Sleep(400);
			EnterCriticalSection(&m_csGDILock);
			LIGHT_WHITE GOTOXY(68,6); printf("┃ '       '을 길게 눌러보세요!   ┃");
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
			LIGHT_WHITE GOTOXY(68,6); printf("┃ '       '을 길게 눌러보세요!   ┃");
			LIGHT_YELLOW GOTOXY(73,6); printf("Key E");
			LeaveCriticalSection(&m_csGDILock);
			PROJECT_1(tu_2_value);
			tu_2_true--;
		}
	}
	Sleep(400);
	EnterCriticalSection(&m_csGDILock);
	LIGHT_WHITE GOTOXY(68,6); printf("                                    ");
	GOTOXY(68,6); printf("┏     듀토리얼을 마치겠습니다    ┓");
	GOTOXY(68,7); printf("┗        입력 시 듀토리얼 종료!  ┛");
	GOTOXY(74,7); LIGHT_AQUA printf("ESC");
	LeaveCriticalSection(&m_csGDILock);
	PROJECT_1(5);
}
 
/* 녹음 시작 */
void START_RECORD(int record_link)
{
	HWAVEIN hWaveIn;		// HANDLE값을 저장
	MMRESULT result = 0;
	WAVEFORMATEX pFormat;	// 구조체 선언
	WAVE_HEADER header;		// wav파일 헤더

	FILE *fp = NULL;
	FILE *bin = NULL;

	const int NUMPTS = sampleRate * duration * channel * (int)(bps / 8);   // signal length
	static time_t start, end;		// 변하면 안되기 때문에 (static)
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
 
	result = waveInOpen(&hWaveIn, WAVE_MAPPER, &pFormat, 0L, 0L, WAVE_FORMAT_DIRECT);		// 녹음하기 위해 사운드드라이버의 HANDLE값을 반환

// Set up an prepare header for input

	WaveInHdr.lpData = (char *)waveIn;
	WaveInHdr.dwBufferLength = NUMPTS * 2;
	WaveInHdr.dwBytesRecorded = 0;
	WaveInHdr.dwUser = 0L;
	WaveInHdr.dwFlags = 0L;
	WaveInHdr.dwLoops = 0L;

	waveInPrepareHeader(hWaveIn, &WaveInHdr, sizeof(WAVEHDR));		// wav파일 헤더와 HANDLE값이 준비됬음을 알림
 
// Insert a wave input buffer
	waveInAddBuffer(hWaveIn, &WaveInHdr, sizeof(WAVEHDR));

	if(record_link == 0)		// 녹음 시작
	{
		start = time(NULL);

		result = waveInStart(hWaveIn);		// 녹음 시작
		GOTOXY(80,4); LIGHT_WHITE printf("   F1 녹음 재개      F2 녹음 중지 / 일시 정지      F3 녹음 파일 저장      Esc 나가기\n");
		GOTOXY(80,4); LIGHT_YELLOW printf("   F1 녹음 재개");
		GOTOXY(55,10);GREEN printf("♬  R E C O R D I N G  ♬");
		
		PROJECT_1(10);
	}
	if(record_link == 1)		// 녹음 일시 정지
	{
		end = time(NULL);
		pst = difftime(end,start);		// 녹음과 일시 정지 사이의 시간
		ttt+=pst;						// 총 녹음 시간
		waveInStop(hWaveIn);			// 녹음 정지

		bin = fopen("c:\\Project\\system\\test.bin", "ab+");		// 이진 파일 생성 및 추가 쓰기 모드
	
		for (i = 0; i < sampleRate * pst * (int)(bps / 8); i++)		// 녹음한 내용 이진파일에 작성
			fwrite(&waveIn[i], sizeof(short), 1, bin);
		
		fclose(bin);

		count_record = 1;		// esc로 종료시 이진파일 삭제
		GOTOXY(80,4); LIGHT_WHITE printf("   F1 녹음 재개");
		GOTOXY(101,4); LIGHT_YELLOW printf("F2 녹음 중지 / 일시 정지");
		GOTOXY(55,10);GREEN printf("♬      P A U S E      ♬");
		if(record_value == 1)			// 일시정지 하고 다시 녹음을 재개하고 다시 일시정지 했을 때,
		{
			i=0;
			while(1)
			{
			GOTOXY(49,40); LIGHT_AQUA printf("파일을 저장하고 있습니다. 잠시만 기다려주세요.    ");
			Sleep(700);
			GOTOXY(49,40); LIGHT_AQUA printf("파일을 저장하고 있습니다. 잠시만 기다려주세요. .  ");
			Sleep(700);
			GOTOXY(49,40); LIGHT_AQUA printf("파일을 저장하고 있습니다. 잠시만 기다려주세요. . .");
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

		

	if(record_link == 2)		// 녹음 파일 저장
	{
		bin = fopen("c:\\Project\\system\\test.bin", "rb");		// 이진파일 읽기모드로 열기

		i=0;

		while(1)		// wavein배열에 녹음파일 내용 전달
		{
			bytes = fread(&waveIn[i], sizeof(short), 1, bin);
			if(bytes <= 0)
				break;
			i++;
		}

		GOTOXY(101,4); LIGHT_WHITE printf("F2 녹음 중지 / 일시 정지");
		GOTOXY(131,4); LIGHT_YELLOW printf("F3 녹음 파일 저장");
		GOTOXY(55,40); LIGHT_AQUA printf(" 저장할 파일 이름 입력 : ");
		LIGHT_YELLOW gets(file_name);
		sprintf(file_name_wav, "%s%s%s", RECORD_LINK, file_name, FILE_EXTENSION);		// 문자열 조합

		fopen_s(&fp, file_name_wav, "wb");		// wav 파일 형식으로 쓰기모드로 열기

		/* wav 헤더 초기화 */
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
 
		fwrite(&header, sizeof(header), 1, fp);		// wav 헤더 정보를 저장
 
		for (i = 0; i < sampleRate * ttt * (int)(bps / 8); i++)		// 녹음 내용 뒤에 wav 헤더 정보를 이음
			fwrite(&waveIn[i], sizeof(short), 1, fp);

		waveInStop(hWaveIn);		// 녹음 정지

		/* HANDLE 및 buffer , 각종 초기화 했던 정보들을 반환 */
		waveInReset(hWaveIn);		
		result = waveInUnprepareHeader(hWaveIn, &WaveInHdr, sizeof(WAVEHDR));
		result = waveInClose(hWaveIn);		 

		fclose(fp);
		fclose(bin);

		count_record = 0;
		record_value = 0;
		ttt = 0;
		remove("c:\\Project\\system\\test.bin");
		IMMEDIATELY_PLAY(file_name_wav);		// 바로 듣기 함수 호출
		system("cls");
		PLAY_LP(0);
	}
}

/* 즉시 듣기 */
void IMMEDIATELY_PLAY(char record_file_name[23])		// 녹음 파일 이름을 매개변수로 받아옴
{
	char file_name_play[50];
	char file_name_stop[50];
	char file_name_pause[50];
	char file_name_resume[50];
	char file_name_close[50];
	int record_key, record_true, record_value;

	record_value = 0;
	record_true = 1;

	/* 문자열 조합 */
	sprintf(file_name_play, "%s%s", MUSIC_PLAY ,record_file_name);
	sprintf(file_name_stop, "%s%s", MUSIC_STOP ,record_file_name);
	sprintf(file_name_pause, "%s%s", MUSIC_PAUSE, record_file_name);
	sprintf(file_name_resume, "%s%s", MUSIC_RESUME, record_file_name);
	sprintf(file_name_close, "%s%s", MUSIC_CLOSE, record_file_name);

	system("cls");
	GOTOXY(1,3);
	LIGHT_RED 
	printf("    ┏━┓ ┏━━ ┏━━ ┏━┓ ┏━┓  ━┓ ━┳━ ┏┓  ┓ ┏━━┓\n");
	printf("    ┣┳┛ ┣━━ ┃     ┃  ┃ ┣┳┛ ┃ ┃   ┃   ┃┗┓┃ ┃  ┏┓\n");
	printf("    ┃┗━ ┗━━ ┗━━ ┗━┛ ┃┗━  ━┛ ━┻━ ┗  ┗┛ ┗━━┛\n");
	GOTOXY(1,8);
	LIGHT_WHITE printf("=========================================================================================================================================================================\n\n\n\n");
	GOTOXY(140,41);
	printf("- product by 유 1 무 2");
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
				SELECT_RECORD_1(record_value);		// 조건에 맞는 함수 호출
			}
			else if(record_value == 3 || record_value == 4 || record_value == 5)
			{
				record_value--;
				if(record_value < 3)
					record_value = 5;
				SELECT_RECORD_3(record_value);		// 조건에 맞는 함수 호출
			}
			break;
		case 80 :			// DOWN
			if(record_value == 0 || record_value == 1)
			{
				record_value++;
				if(record_value > 1)
					record_value = 0;
				SELECT_RECORD_1(record_value);		// 조건에 맞는 함수 호출
			}
			else if(record_value == 3 || record_value == 4 || record_value == 5)
			{
				record_value++;
				if(record_value > 5)
					record_value = 3;
				SELECT_RECORD_3(record_value);		// 조건에 맞는 함수 호출
			}
			break;
		case 13 :
			if(record_value == 0)				// ENTER
			{
				mciSendString(file_name_play, NULL, 0, NULL);		// 녹음 파일 재생
				record_value = 2;
				SELECT_RECORD_2();
			}

			else if(record_value == 1)			// 함수 탈출
				record_true--;

			else if(record_value == 3)	
			{
				mciSendString(file_name_resume, NULL, 0, NULL);		// 녹음 파일 일시 정지
				SELECT_RECORD_2();
			}
			else if(record_value == 4)
			{
				mciSendString(file_name_stop, NULL, 0, NULL);		// 녹음 파일 정지
				mciSendString(file_name_play, NULL, 0, NULL);		// 녹음 파일 시작  --> 처음부터 다시 재생
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

/* 즉시 듣기 관련 인터페이스 1 */
void SELECT_RECORD_1(int sel_rec_value_1)
{
	int sel_rec_1;
	int sel_rec_x = 64;
	int sel_rec_y[] = {19,20,21,22,23, 26,27,28,29};
	char print_m[9][120] = {"      ┓ ━━┓    ┏━         ┓     ",
						    "┣━┫┣ ┏━┛    ┗━   ━━┓┃",
						    "┗━┛┃ ┗━━   ━━━      ┃┃",
							"         ━┻━    ┏━         ┃       ",
                            "                   ┗━        ",

						    " ━━┓  ┓┃ ┏┻┓          ┓       ┓    ",
						    "     ┃━┃┃ ┛  ┗   ━┻━ ┃ ━━┓┃  ",
						    "     ┃━┃┃ ━┻━   ┏━┓ ┣     ┃┃   ",
						    "         ┃┃   ━┓   ┗━┛ ┃       ┃     "};
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
		GOTOXY(sel_rec_x-9,sel_rec_y[2]);LIGHT_YELLOW printf("━━▶");
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
		GOTOXY(sel_rec_x-9,sel_rec_y[7]);LIGHT_YELLOW printf("━━▶");
	}

}
/* 즉시 듣기 관련 인터페이스 2 */
void SELECT_RECORD_2()
{
	system("cls");
	GOTOXY(1,3);
	LIGHT_RED 
	printf("    ┏━┓ ┏━━ ┏━━ ┏━┓ ┏━┓  ━┓ ━┳━ ┏┓  ┓ ┏━━┓\n");
	printf("    ┣┳┛ ┣━━ ┃     ┃  ┃ ┣┳┛ ┃ ┃   ┃   ┃┗┓┃ ┃  ┏┓\n");
	printf("    ┃┗━ ┗━━ ┗━━ ┗━┛ ┃┗━  ━┛ ━┻━ ┗  ┗┛ ┗━━┛\n");
	GOTOXY(1,8);
	LIGHT_WHITE printf("=========================================================================================================================================================================\n\n\n\n");
	GOTOXY(140,41);
	printf("- product by 유 1 무 2");

	GREEN GOTOXY(51,21); printf("♬   ┏━┓┃     ┏━━┓┃      ┃ ━┳━ ┏┓  ┃ ┏━━━┓   ♬  \n");
	GREEN GOTOXY(51,22); printf("♬   ┣━┛┃     ┣━━┫┗┓  ┏┛   ┃   ┃┗┓┃ ┃  ┏━┓   ♬  \n");
	GREEN GOTOXY(51,23); printf("♬   ┃    ┃     ┃    ┃  ┗┳┛     ┃   ┃  ┃┃ ┃  ┗  ┃   ♬  \n");
	GREEN GOTOXY(51,24); printf("♬   ┃    ┗━━ ┃    ┃    ┃     ━┻━ ┃  ┗┛ ┗━━━┛   ♬  \n");
	LIGHT_AQUA GOTOXY(78,27); printf(" E S C 나가기");
}
/* 즉시 듣기 관련 인터페이스 3 */
void SELECT_RECORD_3(int sel_rec_value_3)
{
	int sel_rec_3;
	int sel_rec_x = 58;
	int sel_rec_y[] = {16,17,18,19,20, 23,24,25,26,27,  30,31,32,33,34};
	
	char print_m[15][120] = {"━━┓  ┓┃ ┏┻┓    ━┳┳━ ┓┃   ┃  ┓┃",
						     "    ┃━┃┃ ┛  ┗    ┏┛┗┓ ┣┫ ┏┻┓┣┫",
						     "    ┃━┃┃ ━┻━    ┛    ┗ ┃┃ ┛  ┗┃┃",
						 	 "        ┃┃   ━┓                       ━┓",
							 "                                         ┗━ ",
						 	 "       ┓         ┓   ━┳┳━ ┓┃   ┃  ┓┃",
							 "┏━━ ┃  ┏┻┓ ┃   ┏┛┗┓ ┣┫ ┏┻┓┣┫",
							 "┃     ┣  ┛  ┗ ┃   ┛    ┗ ┃┃ ┛  ┗┃┃",
							 "┗━━ ┃         ┃                      ━┓",
							 "                                         ┗━",
							 "               ┓          ┓       ┓",
						     " ━━┓  ┏━┓┃   ━┻━ ┃ ━━┓┃",
						     "     ┃  ┗━┛┣   ┏━┓ ┣     ┃┃",
							 "     ┃        ┃   ┗━┛ ┃       ┃",
							 "━━━━   ┗━━"};

	char *print_n[] = { print_m, print_m + 1, print_m + 2, print_m + 3, print_m + 4, print_m + 5, print_m + 6, print_m + 7, print_m + 8, print_m + 9, print_m + 10, print_m + 11, print_m + 12, print_m + 13, print_m + 14 };

	system("cls");
	GOTOXY(1,3);
	LIGHT_RED 
	printf("    ┏━┓ ┏━━ ┏━━ ┏━┓ ┏━┓  ━┓ ━┳━ ┏┓  ┓ ┏━━┓\n");
	printf("    ┣┳┛ ┣━━ ┃     ┃  ┃ ┣┳┛ ┃ ┃   ┃   ┃┗┓┃ ┃  ┏┓\n");
	printf("    ┃┗━ ┗━━ ┗━━ ┗━┛ ┃┗━  ━┛ ━┻━ ┗  ┗┛ ┗━━┛\n");
	GOTOXY(1,8);
	LIGHT_WHITE printf("=========================================================================================================================================================================\n\n\n\n");
	GOTOXY(140,41);
	printf("- product by 유 1 무 2");

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
		GOTOXY(sel_rec_x-9,sel_rec_y[2]);LIGHT_YELLOW printf("━━▶");
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
		GOTOXY(sel_rec_x-9,sel_rec_y[7]);LIGHT_YELLOW printf("━━▶");
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
		GOTOXY(sel_rec_x-9,sel_rec_y[12]);LIGHT_YELLOW printf("━━▶");
	}
}

/* 프로젝트 선택 관련 함수 */
void Project_select(int pro_sel_num){
   char project[5][50]={"project1\\", "project2\\", "project3\\", "project4\\", "project5\\"}; // 경로 지정
   //프로젝트 음원들의 파일명 지정
   char Project1[][100]={"a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7", "a8", "a9", "a10", "a11", "a12", "a13", "a14", "a15", "a16", "a17", "a18", "a19", "a20", "a21", "a22", "a23", "a24", "a25", "a26", "a27", "a28", "a29"};
   char Project2[][100]={"a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7", "a8", "a9", "a10", "a11", "a12", "a13", "a14", "a15", "a16", "a17", "a18", "a19", "a20", "a21", "a22", "a23", "a24", "a25", "a26", "a27", "a28", "a29"};
   char Project3[][100]={"a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7", "a8", "a9", "a10", "a11", "a12", "a13", "a14", "a15", "a16", "a17", "a18", "a19", "a20", "a21", "a22", "a23", "a24", "a25", "a26", "a27", "a28", "a29"};
   char Project4[][100]={"a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7", "a8", "a9", "a10", "a11", "a12", "a13", "a14", "a15", "a16", "a17", "a18", "a19", "a20", "a21", "a22", "a23", "a24", "a25", "a26", "a27", "a28", "a29"};
   char Project5[][100]={"a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7", "a8", "a9", "a10", "a11", "a12", "a13", "a14", "a15", "a16", "a17", "a18", "a19", "a20", "a21", "a22", "a23", "a24", "a25", "a26", "a27", "a28", "a29"};
   
   int i=0;
   int pro1_num=0, pro2_num=0, pro3_num=0, pro4_num=0, pro5_num=0;

   system("cls");

   if(pro_sel_num==1){ // 사용자가 1번프로젝트를 선택한 경우
      pro1_num=sizeof(Project1)/100; // 프로젝트 음원 파일의 갯수를 구함
      for(i=0; i<pro1_num; i++){
      //1번 프로젝트에 해당하는 파라메터 생성
      sprintf(Sel_project_start[i], "%s%s%s%s%s", MUSIC_PLAY, LINK, project[0],  Project1[i], FILE_EXTENSION); // "Play+ 파일이름+확장자" 문자열 조합
      sprintf(Sel_project_stop[i], "%s%s%s%s%s", MUSIC_STOP, LINK, project[0],  Project1[i], FILE_EXTENSION); // "Stop+파일이름+확장자" 문자열 조합
      }
      return;
   }

   if(pro_sel_num==2){ // 사용자가 2번프로젝트를 선택한 경우
      pro2_num=sizeof(Project2)/100;
      for(i=0; i<pro2_num; i++){
      //2번 프로젝트에 해당하는 파라메터 생성
      sprintf(Sel_project_start[i], "%s%s%s%s%s", MUSIC_PLAY, LINK, project[1],  Project2[i], FILE_EXTENSION); // "Play+ 파일이름+확장자" 문자열 조합
      sprintf(Sel_project_stop[i], "%s%s%s%s%s", MUSIC_STOP, LINK, project[1],  Project2[i], FILE_EXTENSION); // "Stop+파일이름+확장자" 문자열 조합
      }
      return;
   }

   if(pro_sel_num==3){ // 사용자가 3번프로젝트를 선택한 경우
      pro3_num=sizeof(Project3)/100;
      for(i=0; i<pro3_num; i++){
      //3번 프로젝트에 해당하는 파라메터 생성
      sprintf(Sel_project_start[i], "%s%s%s%s%s", MUSIC_PLAY, LINK, project[2],  Project3[i], FILE_EXTENSION); // "Play+ 파일이름+확장자" 문자열 조합
      sprintf(Sel_project_stop[i], "%s%s%s%s%s", MUSIC_STOP, LINK, project[2],  Project3[i], FILE_EXTENSION); // "Stop+파일이름+확장자" 문자열 조합
      }
      return;
   }

   if(pro_sel_num==4){ // 사용자가 4번프로젝트를 선택한 경우
      pro4_num=sizeof(Project4)/100;
      for(i=0; i<pro4_num; i++){
      //4번 프로젝트에 해당하는 파라메터 생성
      sprintf(Sel_project_start[i], "%s%s%s%s%s", MUSIC_PLAY, LINK, project[3],  Project4[i], FILE_EXTENSION); // "Play+ 파일이름+확장자" 문자열 조합
      sprintf(Sel_project_stop[i], "%s%s%s%s%s", MUSIC_STOP, LINK, project[3],  Project4[i], FILE_EXTENSION); // "Stop+파일이름+확장자" 문자열 조합
      }
      return;
   }

   if(pro_sel_num==5){ // 사용자가 5번프로젝트를 선택한 경우
      pro5_num=sizeof(Project5)/100;
      for(i=0; i<pro5_num; i++){
      //5번 프로젝트에 해당하는 파라메터 생성
      sprintf(Sel_project_start[i], "%s%s%s%s%s", MUSIC_PLAY, LINK, project[4],  Project5[i], FILE_EXTENSION); // "Play+ 파일이름+확장자" 문자열 조합
      sprintf(Sel_project_stop[i], "%s%s%s%s%s", MUSIC_STOP, LINK, project[4],  Project5[i], FILE_EXTENSION); // "Stop+파일이름+확장자" 문자열 조합
      }
      return;
   }
}