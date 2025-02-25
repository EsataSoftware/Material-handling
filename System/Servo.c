#include "Servo.h"

int Grab_falg = 0;
#define Cloud_Location    1080 // 云台观察位置//1080 1700
#define Could_Palace      2300 // 云台放置载物台上位置//2300
#define stage_datum_point 1200 // 载物台初始位置
#define stage_state1      600  // 载物台物块1放置
#define stage_state2      970  // 载物台物块2放置
#define stage_state3      1710 // 载物台物块3放置
#define claw_grab         280  // 机械爪抓取
#define claw_free         500  // 机械爪释放
#define cloud_place       2250
/// 1载物330，2载物970，3载物1710，抓取330 970 1710
unsigned int claw_poriect_free = 700;

void Servo_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    TIM_InternalClockConfig(TIM1);
    //
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    TIM_TimeBaseInitStructure.TIM_ClockDivision     = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode       = TIM_CounterMode_Up;
    TIM_TimeBaseInitStructure.TIM_Period            = 20000 - 1; //       //PWM频率 = 72M/72/20000 50hz
    TIM_TimeBaseInitStructure.TIM_Prescaler         = 72 - 1;    // 占空比=CCR/ARR
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseInitStructure);

    TIM_OCInitTypeDef TIM_OCInitStructure;
    TIM_OCStructInit(&TIM_OCInitStructure);
    TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Set;
    TIM_OCInitStructure.TIM_OCMode      = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OCPolarity  = TIM_OCPolarity_High;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse       = claw_free;
    TIM_OC2Init(TIM1, &TIM_OCInitStructure);
    TIM_OCInitStructure.TIM_Pulse = stage_state1;
    TIM_OC3Init(TIM1, &TIM_OCInitStructure);
    TIM_OCInitStructure.TIM_Pulse = Cloud_Location;
    TIM_OC4Init(TIM1, &TIM_OCInitStructure);

    TIM_CtrlPWMOutputs(TIM1, ENABLE);
    //	TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);
    TIM_OC2PreloadConfig(TIM1, TIM_OCPreload_Enable);
    TIM_OC3PreloadConfig(TIM1, TIM_OCPreload_Enable);
    TIM_OC4PreloadConfig(TIM1, TIM_OCPreload_Enable);

    TIM_Cmd(TIM1, ENABLE);
}

void PWM1_SetCompare2(uint16_t Compare) // PA9机械爪
{
    TIM_SetCompare2(TIM1, Compare);
}

void PWM1_SetCompare3(uint16_t Compare) // PA10载物台
{
    TIM_SetCompare3(TIM1, Compare);
}

void PWM1_SetCompare4(uint16_t Compare) // PA11云台
{
    TIM_SetCompare4(TIM1, Compare);
}

void cloud_tai(int site) // 770 1950
{

    static int now_site = cloud_place;
    if (now_site < site) {
        for (; now_site < site; now_site += 10) // 云台从观察位置转至载物台位置
        {
            PWM1_SetCompare4(now_site);
            Delay_ms(20);
        }
    } else if (now_site > site) // 云台从载物台位置转至观察位置
    {
        for (; now_site > site; now_site -= 10) {
            PWM1_SetCompare4(now_site);
            Delay_ms(20);
        }
    }

    now_site = site;
    PWM1_SetCompare4(now_site);
    Delay_ms(50); // 对于不够精准的数值进行回归精准值
}

void robotic_grab(int site) // 890 760 500 claw_grab抓取 claw_free释放  claw_look初始位置方便openmv观察
{
    static int now_site = claw_grab; //
    if (now_site < site) {
        for (; now_site < site; now_site = now_site + 10) // 机械爪从释放状态转为抓取状态
        {

            PWM1_SetCompare2(now_site);
            Delay_ms(20);
        }
        PWM1_SetCompare2(claw_poriect_free);
        Delay_ms(50);           // 对于不够精准的数值进行回归精准值
    } else if (now_site > site) //
    {

        for (; now_site > site; now_site = now_site - 10) {
            PWM1_SetCompare2(now_site);
            Delay_ms(20);
        }
    }
    now_site = claw_grab;

    PWM1_SetCompare2(500);
    Delay_ms(1000);
}
