# C based launchpad program

- 연구 기간 : 2016.03.02 ~ 2016.06.20 <br>
- 연구 참여자 : <br>
  - 20130999 김준호(팀원 협동 추진 및 전체적 검토) <br>
  - 20130997 김정승(기능적인 요소 구현 및 코딩 구상) <br>
  - 20151016 염준섭(API함수자료 조사 및 실험)[군입대] <br>
  - 20150997 김효권(simulation 및 성능개선) <br>
- 지도 교수 : 정복래 교수님
<br>

## 목차

1. 설계과제 제목

2. 연구목적

3. 설계과제의 필요성

4. 설계과제의 목표

5. 설계과정

6. 제작

7. 테스트

8. 설계 추진일정

9. 결론
<br>

## 본문

### 1. 설계과제 제목
 - C언어를 활용한 런치패드 구현

### 2. 연구목적
 1)  c언어 상 여러 가지 기능 및 함수 이해
 2)  주변 전자기기를 c언어를 활용해 구현할 수 있는 능력 고취
 3)  하드웨어를 통해 소프트웨어를 제어하는 기능 이해
 4)  HANDLE을 이용한 low-level 프로그래밍

### 3. 설계과제의 필요성 
 - c언어의 실력 증진을 위해.
 - 실무적인 측면에서의 이해력 증진
 - 팀원 간 단합 및 협업의 중요성
 
### 4. 설계과제의 목표
본 과제에서 달성하고자 하는 런치 패드에 대한 성능 목표는 다음과 같으며, 이외에도 본 과제에서 표 1과 같은 현실적인 제한요소들은 달성한다.  
- 구현하고자 하는 기능 <br>
ㆍ 음악 재생과 정지 및 반복
ㆍ 키보드 입력 감지
ㆍ 녹음 및 불러오기
ㆍ 튜토리얼 기능
<br>

표 1. 본 프로젝트의 현실적 제한요소 항목
|제목|내용|
|---|---|
|경 제| - 실제 기기가 경제적인 소요가 많아서, 개발 프로그램을 통해 경제적 절약 효과 기대 |
|편 리|- 처음 사용하는 유저에게도 쉽게 다가갈 수 있도록 graphic interface를 구현 <br> - 런치패드의 버튼을 흔히 사용하는 키보드로 대체 해 구현 <br> - 콘솔상의 제한적인 부분(키보드를 누를 때의 표현)을 색깔변화로 표현|
|윤 리|- 개발프로그램에 들어가는 MP3 파일을 남용해 인터넷 유포에 사용하지 말 것.|
|사 회|- 실제 기기를 사용하지 못했던 사람들에게 미흡하게나마 대체 품목으로 사용할 수 있어서, 사회적으로 음악에 대한 접근을 극대화|


### 5. 설계과정

5.1 설계 기초이론 <br>
- 5.1.1 참고서적 
  - API함수
    - www.devpia.com (데브피아)
    - https://msdn.microsoft.com/ko-kr/dn308572.aspx (MSDN)
    - http://crystalcube.co.kr/123 
    - http://forum.falinux.com/zbxe/index.php?document_srl=406372&mid=graphic (Wav 헤더 관련)
    - http://bluelimn.tistory.com/entry/API-waveInOpen (녹음 관련)
    - http://blog.naver.com/rains1230/220061356062 (파일 불러오기 관련)
    - http://cafe.naver.com/cafec/241262 (키 감지 관련) <br>
  - 쓰레드
    - http://cafe.naver.com/cafec/354115
    - http://egloos.zum.com/newkong/v/4006039
 
  
 - 파일 입출력
    - 4주차 수업자료 (이진 파일 관련)

- 5.1.2 소프트웨어 기능블록도 (개념설계) <br>
![블록도](/img/런치패드-블록도.png)

- 5.1.3 기능적 요구사항 명세서 

|요구사항|내용|설명|우선순위|
|---|---|---|---|
|음악 파일 재생 및 정지| 이용자가 키보드 입력으로 음악을 제어한다. | 이용자가 키를 입력했을 때 음악 재생, 키를 떼어냈을 때 음악 정지 기능 구현 | 2 |
|키보드 입력 감지|입력자가 키보드 입력으로 프로그램을 제어한다. |키보드 입력으로 함수를 호출하여 입력으로 음악을 제어하고 입력으로 프로그램을 종료한다. |1|
|녹음 및 불러오기|이용자가 믹싱한 사운드를 녹음한다.|이용자가 연주하는 도중에 녹음하기를 원한다면 키 입력으로 녹음을 한다. 그리고 녹음파일을 불러올 수 있다.|5|
|튜토리얼|이용자가 튜토리얼을 이용할 수 있다.|이용자가 특정 키를 입력하면 튜토리얼이 시작된다. 프로그램 처음 이용시 도움을 준다.|3|
|시각적인 표현|이용자가 시각적으로 흥미를 느낀다.|프로젝트 파일을 변경할 때나 음악 변경 시, 애니메이션 또는 색상으로 시각적인 효과를 준다.|4|

- 5.1.4 전체/부분 순서도 (상세설계)
  - 개략 순서도 (1/1) <br>
  ![순서도01](/img/순서도01.png)
    - 배경음악을 mciSendString을 이용해서 재생
    - system() 및 removecusor() 함수를 사용해서 콘솔창 및 커서 제어
    - 인터페이스를 출력 후, 메뉴 함수를 불러온다
   
  - 상세 순서도 (1/6) <br>
  ![순서도02](/img/순서도02.png)
    - 키 입력을 받고 그 입력 값(조건)에 따라서 해당 순서를 진행
    - key == 72 일 경우, 방향키 ↑와 연동, 각각 80, 13은 ↓, ENTER와 연동
    - i 값에 따라서 불러오는 함수가 달라짐
    
  - 상세 순서도 (2/6) <br>
  ![순서도03](/img/순서도03.png)
    - 키 입력을 받고 그 입력 값(조건)에 따라서 해당 순서를 진행
    - 방향키 ↑와 ↓을 이용하여 프로젝트(음원) 선택
    - ENTER 키를 이용해 해당 프로젝트 변경

  - 상세 순서도 (3/6) <br>
  ![순서도04](/img/순서도04.png)
    - 키 입력을 받고 그 입력 값(조건)에 따라 추가 기능 함수 및 스레드 실행
    
  - 상세 순서도 (4/6) <br>
  ![순서도05](/img/순서도05.png)
    - 스레드 진입 시 키(Key)의 상태 값을 저장할 전역변수 사용
    - 각각의 스레드의 조건(키 입력)에 따라 button애니메이션 및 해당 사운드 출력 
 
  - 상세 순서도 (5/6) <br>
  ![순서도06](/img/순서도06.png)
    - 키 입력을 받고 그 입력 값(조건)에 따라서 해당 순서를 진행
    - 방향키 ↑와 ↓을 이용하여 프로젝트(음원) 선택
    - ENTER 키를 이용해 해당 프로젝트 변경
 
  - 상세 순서도 (6/6) <br>
  ![순서도07](/img/순서도07.png)
    - WaveIn함수를 통한 녹음장치(사운드카드)에 대한 HANDLE 값 획득
    - WAVEHDR구조체와 WaveInPrepareHeader함수를 통해 녹음에 필요한 버퍼가 
    준비되었음을 드라이버에 전달
    - WaveInStart함수를 통해 녹음진행, F2키를 통한 녹음 종료
    - WaveOutOpen함수를 통한 재생장치에 대한 HANDLE 값 획득
    - WaveOutPrepareHeader함수와 WaveoutWrite함수를 통한 녹음 데이터 출력

### 6. 제작(Implementation)
6.1 제작과정 <br>
- 핵심함수
  1. unsigned __stdcall T1(void *arg) ~ unsinged __stdcall T29(void *arg)
    -> 사용자가 키보드를 입력할 시 키보드를 누른 상태일 경우 음원파일 재생 및           button 애니메이션 출력, 사용자가 키보드를 누르지 않고 있을 경우 음원파일         정지 및 button 애니메이션 출력 
   2. main(void)
    -> 프로그램 콘솔 창 제어, 배경음악 재생, interface화면 출력 및 MENU함수 
       호출
   3. MENU()
     -> 사용자가 입력한 키보드에 따라 화면을 출력하며, Enter키를 눌렀을 때 
        조건에 맞는 함수(PLAY_LP, RECORD_LOAD) 호출
   
   4. void PLAY_LP(int pl_link)
      -> 콘솔 화면상에 실제 런치패드 화면을 시각적으로 표현, 조건식에 따라 
         TUTO_2, PROJECT_1, SELECT_LP_1 함수 호출
   5. void PROJECT_1(int pr_1_link)
       -> 사용자가 입력 한 키에 따른 스레드(T0~T29) 실행, ESC키를 눌렀을 시
          MENU함수 호출, 방향키(→) 입력 시 해당 함수 종료, F1~F3키에 따라
          START_RECORD, TUTO_1함수 호출  
   6. void SELECT_LP_1(int sel_lp_1_link_1, int sel_lp_1_link_2)
        -> 사용자가 프로젝트 음원을 선택할 수 있도록 하는 함수
           화살표 키를 통해 프로젝트를 선택 하면 Project_select()함수 호출 
   7. void TUTO_1(), void TUTO_2()
        -> 사용자에게 튜토리얼 기능을 시각적으로 표현, 사용자의 키보드 입력에
           따라 조건식을 만족할 시 함수 진행 
   8. void START_RECORD(int record_link)
        -> WAVEFORMATEX pFormat 형식을 통한 녹음포맷 지정, 
           WAVE_HEADER header 형식을 통한 wav헤더형식 지정,
           waveIn함수를 통한 사운드 장치 핸들값 획득, 녹음메모리 설정 및 
           녹음 실행, 정지 
   9. void RECORD_LOAD(void)
         -> FindFirstFile, FindNextFile 함수를 통한 녹음폴더 내의 녹음파일 데이터
            저장. 녹음파일 데이터를 시각적으로 콘솔 화면에 표현, 실제 녹음파일 
            재생

- 라이브러리 함수
    1. mciSendString
         -> 해당 함수의 파라메터로 실제 재생되어질 음원파일의 
            “명령(Play,Stop)+경로+파일명”을 넘겨주어 원하는 음원파일 재생
    2. _beginthreadex
          -> 각 키마다 스레드를 배정하여 키를 입력했을 시 조건에 맞는 스레드 
              실행 
    3. sprintf
          -> mciSendString의 파라메터로 넘겨 줄 문자열(명령+경로+파일명) 
             수정에 사용  
    4. GetAsyncKeyState
          -> 해당 함수의 반환값(0x8000)을 통한 키보드 입력 감지
    5. Sleep
          -> 프로그램상의 시간적 딜레이가 필요한 부분에 딜레이 적용
    
    6. system
          -> 화면 지우기(cls) 및 콘솔창 설정(mode)
    7. fopen, fwrite, fclose
          -> 녹음파일의 임시 파일(test.bin) 생성 및 실제 wav파일 생성에 사용 
    8. time, difftime
          -> 녹음 시작과 녹음 종료사이의 시간을 구하기 위해 사용

6.2 제작시 문제점 및 개선사항 <br>
1. 키보드 동시입력
- 런치패드의 특성상 여러 개의 키 입력 시 동시에 음원이 출력되어야 함, 하지만 C언어의 특성상 일반적인 형식으로는 동시출력이 불가하였으며, 각각의 키의 스레드를 생성하여 해결함. 
2. 키보드 입력감지
- 사용자가 키보드를 입력하고 있을 시(누르고 있음) 해당 음원이 계속 재생되어야 하며, 사용자가 키보드를 입력하고 있지 않은 시(누르고 있지않음) 해당 음원 재생이 중지 되어야 함. 
- 키보드 입력을 감지하는 GetAyncKeyState함수를 통해 해결
3. 스레드 동기화
- 스레드가 여러 개 동작 되었을 때, 스레드 간의 변수의 공유와 함수 접근에 대한 충돌 발생(그래픽 깨짐, 의도치 않은 출력 값 발생 등).
- 임계점(CriticalSection)을 생성하여 동기화 과정 진행하여 해결 함.
4. 녹음
- 녹음시작 후 녹음 일시정지를 하고 녹음을 다시 진행하였을 시 녹음 버퍼의 데이터가 중복이 되어 의도치 않은 녹음 결과 값 출력. 이진파일 입출력을 통해 일시 정지하였을 때 버퍼내의 데이터 값을 임시파일(test.bin)에 저장하여 최종적으로 임시파일의 데이터를 저장하는 과정을 통해 해결함.     
5. 녹음 사운드
- 실제 프로그램 상에서 출력되는 사운드와 출력 사운드를 녹음한 녹음파일의 볼륨(Volume)값 불일치(녹음사운드가 작아짐).
- Windows 제어판 녹음장치의 스테레오 믹스의 볼륨 값을 조절하여 해결 함.
     
6.3 지도교수 지도내용 및 지적사항 조치결과
|일정|지적사항|조사결과|
|---|---|---|
|2016.04.18 ~ 2016.04.24| 프로그램 기능 블록도에 불필요한 부분을 수정하여 좀 더 간략하게 표시할 것 | 기능 블록도를 간략하게 수정하여 다음번 발표내용에 추가하였음 |
|2016.05.09 ~ 2016.05.15| 녹음하는 과정에서 녹음의 일시정지 가능 여부 |임시파일(test.bin)에 버퍼내의 데이터를 저장하는 방식으로 녹음 일시정지 기능을 구현하였음 |
|2016.05.16 ~ 2016.05.22|실제 프로그램내의 출력되는 사운드보다 녹음파일에 녹음 된 사운드가 더 작아짐|Windows 제어판 녹음장치의 스테레오믹스의 녹음사운드를 높여주어 녹음된 사운드가 작아지는 문제를 해결하였음|


### 7. 테스트
1. 키보드 동시입력 : 30개의 스레드를 통해 모든 키보드의 스레드를 배정하여 각각의 키 마다 출력이 독립적으로 작동하게 함

|기대결과|실제결과|
|---|---|
|30개의 스레드의 동시 실행 및 음원재생|키보드의 하드웨어적 특성(동시키 입력)에 제약이 없을 경우 원하는 결과값 출력|

2. 녹음 : 실제 프로그램 상에서 출력되는 음원을 버퍼에 저장 및 실제 파일형식(wav)로 저장할 수 있게 함

|기대결과|실제결과|
|---|---|
|사용자입력에 따른 음악 재생결과를 사운드장치를 통해 버퍼에 저장하여 파일형식으로 저장|기대결과와 일치|

3. 동기화 : 여러 개의 스레드 실행 시 스레드간의 충돌 및 의도치 않은 결과값 출력이 없어야 함.

|기대결과|실제결과|
|---|---|
|여러 개의 키를 입력, 스레드가 실행되었을 시 의도치 않은 결과값 출력 없이 프로그램이 진행되어짐|기대결과와 일치|


### 8. 설계 추진 일정: 2016년 3월 ~ 2016년 6월
![일정](/img/설계-추진-일정.png)

### 9. 결론 
실제 설계 제안과정에서 구현하고자 했던 기능을 설계 진행 중 모두 구현하였으며, 최종 단계에서 제안 목표를 모두 달성하였음.
이번 설계를 통해 실제 프로그램을 설계 및 제작, 보고하는 과정에 대해 이해할 수 있는 계기가 되었으며, 조원들 간의 협동을 통한 팀워크 증진을 할 수 있었음.
또, 프로그램의 기능을 실제 구현하는 과정에서는 C언어 동작 방식 및 운영체제의 동작방식 까지 이해할 수 있었고, 수많은 라이브러리 함수를 통해 원하는 결과 값을 얻는 방식을 알 수 있었음.
