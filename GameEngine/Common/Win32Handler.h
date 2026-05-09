#pragma once

/*
 * Win32Handler.h
 * Windows 메시지 콜백 함수인 WndProc 선언을 제공한다.
 *
 * Win32 창으로 들어오는 키보드, 종료, 창 상태 변경 메시지는 이 콜백을 통해
 * 처리된다. 실제 구현은 Win32Handler.cpp에 있으며, 입력 결과는 EngineTypes.h의
 * localKeyState와 videoConfig에 반영된다.
 */

#include "EngineTypes.h"

// Windows가 창 이벤트를 전달할 때 호출하는 메시지 처리 함수.
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
