#include <windows.h>
#include <time.h>
#include "resource.h"

LRESULT CALLBACK MyWNDPROC(HWND hWnd,UINT Message,WPARAM wParam,LPARAM lParam);

typedef struct NODE
{
	int x;
	int y;
	struct NODE *pNext;
	struct NODE *pLast;
}Snake,Apple;

enum Direction {UP,DOWN,LEFT,RIGHT};

Snake *pHead = NULL;
Snake *pEnd = NULL;
enum Direction fx = UP;
Apple apple = {5,5,NULL,NULL};
BOOL bFlag = TRUE;

HBITMAP hBitmap_Background;
HBITMAP hBitmap_Apple;
HBITMAP hBitmap_SnakeBody;
HBITMAP hBitmap_SnakeHead_Up;
HBITMAP hBitmap_SnakeHead_Down;
HBITMAP hBitmap_SnakeHead_Left;
HBITMAP hBitmap_SnakeHead_Right;

void ShowBackground(HDC hdc);
void AddNode(int x,int y);
void ShowSnake(HDC hdc);
void Move();
void ShowApple(HDC hdc);
BOOL IsEatApple();
void NewApple();
BOOL BumpWall();
BOOL EatItself();

int CALLBACK WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow
	)
{
	WNDCLASSEX ex;
	HWND hWnd;
	MSG Msg;
	int i;

	//加载图片
	hBitmap_Background = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_BITMAP1));
	hBitmap_Apple = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_BITMAP2));
	hBitmap_SnakeBody = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_BITMAP3));
	hBitmap_SnakeHead_Up = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_BITMAP5));
	hBitmap_SnakeHead_Down = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_BITMAP7));
	hBitmap_SnakeHead_Left = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_BITMAP6));
	hBitmap_SnakeHead_Right = LoadBitmap(hInstance,MAKEINTRESOURCE(IDB_BITMAP4));

	//初始化蛇身

		AddNode(3,3);
		AddNode(3,4);
		AddNode(3,5);

	srand((unsigned int)time(NULL));

	//1.设计窗口
	ex.style = CS_HREDRAW | CS_VREDRAW;
	ex.cbSize = sizeof(ex);
	ex.cbClsExtra = 0;
	ex.cbWndExtra = 0;
	ex.hInstance = hInstance;
	ex.hIcon = NULL;
	ex.hCursor = NULL;
	ex.hbrBackground = CreateSolidBrush(RGB(0,255,0));
	ex.hIconSm = NULL;
	ex.lpfnWndProc = &MyWNDPROC;		
	ex.lpszMenuName = NULL;
	ex.lpszClassName = "aaa";

	//2.注册
	RegisterClassEx(&ex);

	//3.创建
	hWnd = CreateWindow(ex.lpszClassName,"贪吃蛇",WS_OVERLAPPEDWINDOW,50,30,615,638,
		NULL,NULL,hInstance,NULL);

	//4.显示
	ShowWindow(hWnd,SW_SHOW);

	//设置定时器
	SetTimer(hWnd,1,200,NULL);

	//消息循环
	while(GetMessage(&Msg,NULL,0,0))
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}

	return 0;
}

LRESULT CALLBACK MyWNDPROC(HWND hWnd,UINT Message,WPARAM wParam,LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;

	switch(Message)
	{
	case WM_CLOSE:
		PostQuitMessage(0);
		break;
	case WM_KEYDOWN:
		if(TRUE == bFlag)
		{
			switch(wParam)
			{
			case VK_UP:
				if(fx != DOWN)
				{
					fx = UP;
				}
				break;
			case VK_DOWN:
				if(fx != UP)
				{
					fx = DOWN;
				}
				break;
			case VK_LEFT:
				if(fx != RIGHT)
				{
					fx = LEFT;
				}
				break;
			case VK_RIGHT:
				if(fx != LEFT)
				{
					fx = RIGHT;
				}
				break;
			}

			bFlag = FALSE;
		}
		
		hdc = GetDC(hWnd);
		//显示背景
		ShowBackground(hdc);
		//显示蛇
		ShowSnake(hdc);
		//显示苹果
		ShowApple(hdc);
		ReleaseDC(hWnd,hdc);
		break;
	case WM_PAINT:		//重绘消息
		hdc = BeginPaint(hWnd,&ps);

		//显示背景
		ShowBackground(hdc);
		//显示蛇
		ShowSnake(hdc);
		//显示苹果
		ShowApple(hdc);

		EndPaint(hWnd,&ps);
		break;
	case WM_TIMER:
		hdc = GetDC(hWnd);
		//蛇移动
		Move();

		//判断游戏是否结束
		if(BumpWall() || EatItself())
		{
			KillTimer(hWnd,1);
			MessageBox(hWnd,"真菜  不愧是游戏黑洞","死了吧",MB_OK);
		}

		//判断是否吃到苹果
		if(IsEatApple())
		{
			//长个
			AddNode(-10,-10);
			//新苹果
			NewApple();
		}

		//显示背景
		ShowBackground(hdc);
		//显示蛇
		ShowSnake(hdc);
		//显示苹果
		ShowApple(hdc);

		ReleaseDC(hWnd,hdc);

		bFlag = TRUE;
		break;
	}

	return DefWindowProc(hWnd,Message,wParam,lParam);
}

void ShowBackground(HDC hdc)
{
	//1.创建兼容性DC
	HDC hdcmen = CreateCompatibleDC(hdc);
	//2.给创建的DC选择一张位图
	SelectObject(hdcmen,hBitmap_Background);
	//3.传输
	BitBlt(hdc,0,0,600,600,hdcmen,0,0,SRCCOPY);
	//4.删除兼容性DC
	DeleteDC(hdcmen);
}

void AddNode(int x,int y)
{
	Snake *pTemp = (Snake*)malloc(sizeof(Snake));
	pTemp->x = x;
	pTemp->y = y;
	pTemp->pLast = NULL;
	pTemp->pNext = NULL;

	if(NULL == pHead)
	{
		pHead = pTemp;
	}
	else
	{
		pEnd->pNext = pTemp;
		pTemp->pLast = pEnd;
	}
	pEnd = pTemp;
}

void ShowSnake(HDC hdc)
{
	Snake *pMark = pHead->pNext;
	HDC hdcmen = CreateCompatibleDC(hdc);
	//1.显示蛇头
	switch (fx)
	{
	case UP:
		SelectObject(hdcmen,hBitmap_SnakeHead_Up);
		break;
	case DOWN:
		SelectObject(hdcmen,hBitmap_SnakeHead_Down);
		break;
	case LEFT:
		SelectObject(hdcmen,hBitmap_SnakeHead_Left);
		break;
	case RIGHT:
		SelectObject(hdcmen,hBitmap_SnakeHead_Right);
		break;
	default:
		break;
	}
	BitBlt(hdc,pHead->x*30,pHead->y*30,30,30,hdcmen,0,0,SRCCOPY);
	//2.显示蛇身
	SelectObject(hdcmen,hBitmap_SnakeBody);
	while(pMark != NULL)
	{
		BitBlt(hdc,pMark->x*30,pMark->y*30,30,30,hdcmen,0,0,SRCCOPY);
		pMark = pMark->pNext;
	}

	DeleteDC(hdcmen);
}

void Move()
{
	Snake *pMark = pEnd;
	//1.移动蛇身
	while(pMark != pHead)
	{
		pMark->x = pMark->pLast->x;
		pMark->y = pMark->pLast->y;
		pMark = pMark->pLast;
	}
	//2.移动蛇头
	switch (fx)
	{
	case UP:
		pHead->y--;
		break;
	case DOWN:
		pHead->y++;
		break;
	case LEFT:
		pHead->x--;
		break;
	case RIGHT:
		pHead->x++;
		break;
	default:
		break;
	}
}

void ShowApple(HDC hdc)
{
	HDC hdcmen = CreateCompatibleDC(hdc);
	SelectObject(hdcmen,hBitmap_Apple);
	BitBlt(hdc,apple.x*30,apple.y*30,30,30,hdcmen,0,0,SRCCOPY);
	DeleteDC(hdcmen);
}

BOOL IsEatApple()
{
	if(pHead->x == apple.x && pHead->y == apple.y)
	{
		return TRUE;
	}

	return FALSE;
}

void NewApple()
{
	int x;
	int y;
	Snake *pMark = pHead;
	//1.随机生成坐标   1-18
	do
	{
		x = rand() % 18 + 1;
		y = rand() % 18 + 1;
		//2.苹果不能在蛇身上
		//遍历蛇身 去比较新生成的坐标
		pMark = pHead;
		while(pMark != NULL)
		{
			if(x == pMark->x && y == pMark->y)
			{
				break;
			}
			pMark = pMark->pNext;
		}
	}while(pMark != NULL);

	apple.x = x;
	apple.y = y;
}

BOOL BumpWall()
{
	if(pHead->x == 0 || pHead->x == 19 || pHead->y == 0 || pHead->y == 19)
	{
		return TRUE;
	}

	return FALSE;
}

BOOL EatItself()
{
	Snake *pMark = pHead->pNext;
	while(pMark != NULL)
	{
		if(pHead->x == pMark->x && pHead->y == pMark->y)
		{
			return TRUE;
		}
		pMark = pMark->pNext;
	}

	return FALSE;
}