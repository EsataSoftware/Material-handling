#ifndef __OLED_H
#define __OLED_H

void OLED_Init(void);
void OLED_Clear(void);
void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char);
void OLED_ShowString(uint8_t Line, uint8_t Column, char *String);
void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length);
void OLED_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
void OLED_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
void OLED_ShowChar_high(uint8_t Line, uint8_t Column, char Char);
void OLED_ShowString_high(uint8_t Line, uint8_t Column, char *String);
void OLED_ShowBinNum_high(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
void OLED_ShowHexNum_high(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
void OLED_ShowSignedNum_high(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length);
void OLED_ShowNum_high(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);

void OLED_ShowDecimals(uint8_t Line, uint8_t Column, float Number);

void OLED_ShowDecimals_high(uint8_t Line, uint8_t Column, float Number);//保留两位小数并转换为字符类型进行显示
/*保留两位小数并转换为字符类型进行显示
使用格式为OLED_ShowDecimals(3,2,789.789);表示从第三行第二列显示789.79*/

#endif
