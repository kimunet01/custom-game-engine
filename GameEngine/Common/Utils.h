#pragma once

/*
 * Utils.h
 * 특정 시스템에 묶이지 않는 작은 유틸리티 함수를 모아 두는 파일이다.
 *
 * 현재는 이동량이 프레임마다 과도하게 커지지 않도록 값을 범위 안에 제한하는
 * ClampFloat만 제공한다. 물리/이동/카메라 보정 등 여러 곳에서 재사용할 수 있다.
 */

// value가 minValue보다 작으면 minValue, maxValue보다 크면 maxValue로 잘라낸다.
// 범위 안의 값이면 원래 값을 그대로 반환한다.
inline float ClampFloat(float value, float minValue, float maxValue)
{
    if (value < minValue) {
        return minValue;
    }
    if (value > maxValue) {
        return maxValue;
    }
    return value;
}
