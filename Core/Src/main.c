/* USER CODE BEGIN Header */
/**
  * @Adi:Emre
  * @Soyadi:Maltaş
  * @Tarih:30.06.2022
  *
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "mesaj_degiskenleri.h" //Struct yapısı bulunan başlık dosyası eklendi.


/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

unsigned char gelen_mesaj[16]= {0};

int gelen_veri_adeti=0;

uint8_t binler = 0 , yuzler = 0, onlar = 0, birler = 0;
int toplam=0;
uint32_t sayimin_gonderilcegi_an =0;
uint16_t sayim_degeri=0;
int hatali_mesaj = 0;
int hatasiz_mesaj =0;
uint8_t okuma_yazma=0;
uint8_t olumsuz_durum=0;
char gelen_karakter=0;

unsigned char hata_mesaji[] = "Eksik veya Hatali Tuslama Yaptiniz!!\n";


uint8_t emirNo_hataSayisi = 0;
uint8_t emirAdres_hataSayisi=0;

MesajYapisiTypeDef yapi; //istenilen yapıyı tutacak yapı
MesajYapisiTypeDef gelen_mesaj_yapisi;  //gelen mesajın yapısını tutacak yapı

void GPIO_Config()
{
	//PinD8 -> USART3_TX  &  PinD9 -> USART3_RX    AF7
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN; //D portu clock hattı aktifedildi.

	GPIOD->MODER |= GPIO_MODER_MODE8_1 | GPIO_MODER_MODE9_1; //Pin8-9 Af olarak kullanılacak.
	GPIOD->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR8 | GPIO_OSPEEDER_OSPEEDR9; //Hız max.
	GPIOD->AFR[1] |= (1<<0) | (1<<1) | (1<<2) | (1<<4) | (1<<5) | (1<<6); //Hangi af olduğunu belirttik.(AF7)
}

void USART_Config()
{
	RCC->APB1ENR |= RCC_APB1ENR_USART3EN; //USART3 clock hattı aktif edildi.

	NVIC_EnableIRQ(USART3_IRQn); // USART3 için oluşan kesmelere izin verildi.

	USART3->BRR = 0X1114; //BaudRate hızı: 9600 olarak ayarlandı.

	USART3->CR1 |= USART_CR1_RXNEIE; //Okuma tamponu dolu kesmesi aktif edildi.

	USART3->CR1 |= USART_CR1_TE; //USART3'ün veri gönderim işlevi açık.
	USART3->CR1 |= USART_CR1_RE; //USART3'ün veri alma işlevi açık.

	USART3->CR1 |= USART_CR1_UE; //USART3 çevresel birimi aktif edildi.
}


void TIMER_Config()
{
	RCC->APB2ENR |= RCC_APB2ENR_TIM1EN; //Timer1'in clock hattı aktif edildi.

	TIM1->CR1 |= TIM_CR1_ARPE; //Sayım değeri değişimi sayım tamamlandığında  etkilenecek.
	TIM1->CR1 &= ~TIM_CR1_CMS;  //Sayım yönü  DIR bitinde belirlenecek.
	TIM1->CR1 &= ~TIM_CR1_DIR;  //Sayım yukarı yönlü olacak şekilde ayarlandı.

	TIM1->ARR = 9999;		    //Başlangıçta 10000 adet sayım olacak.(Period degeri)
	TIM1->PSC = 41999;          //Bölme oranı. Sayım  2.5 saniye sürecek.

	TIM1->CR1 |= TIM_CR1_CEN; //Sayım başlatıldı.
}

void mesaj_gonder( volatile char yollanacak_karakter)  //Aldığı karakteri DR registerina yazarak çıkış veren fonksiyon.
{
	while(!(USART3->SR & USART_SR_TXE)); //Bit veri gönderimine hazır mı kontrol ediliyor.

		USART3->DR = yollanacak_karakter;
		USART3->SR &= USART_SR_TXE; //Bit sıfırlanıyor.Yeni gönderime hazır.
}

void yollanacak_mesaj(volatile unsigned char* adres)  //her karakteri dizinin ilgili adresinden 'mesaj yolla' fonksiyonuna gönderiyoruz.
{
	while(*adres)
	{
		mesaj_gonder(*adres);
		adres++;
	}
}

void USART3_IRQHandler()
{
	if(USART3->SR & USART_SR_RXNE)		//Okuma tamponunun boşluğu kontrol ediliyor.
	{
		USART3->SR &= ~USART_SR_RXNE;

		if(USART3->DR != '\r')
		{
			gelen_karakter = USART3->DR;  //Gelen karakter alınıyor.

			switch(gelen_veri_adeti)  //Aşağıda mesaj yapıları sıra ile alınarak karşılaştırılıyor.
			{
				case 0:
					gelen_mesaj_yapisi.baslangic = gelen_karakter;
					if(gelen_mesaj_yapisi.baslangic == yapi.baslangic)
					{
						gelen_mesaj[gelen_veri_adeti] = gelen_karakter;
					}
					else
					{
					olumsuz_durum++;
					}

					break;
				case 1:
					gelen_mesaj_yapisi.gelen_emir_numarasi[0] = gelen_karakter;

					if(gelen_mesaj_yapisi.gelen_emir_numarasi[0] == yapi.gelen_emir_numarasi[0])
					{
						gelen_mesaj[gelen_veri_adeti] = gelen_karakter;
					}
					else
					{
						olumsuz_durum++;
					}

					break;
				case 2:
					gelen_mesaj_yapisi.gelen_emir_numarasi[1] = gelen_karakter;

					if(gelen_mesaj_yapisi.gelen_emir_numarasi[1] == yapi.gelen_emir_numarasi[1])
					{
						gelen_mesaj[gelen_veri_adeti] = gelen_karakter;
					}
					else
					{
						olumsuz_durum++;
					}

					break;
				case 3:
					gelen_mesaj_yapisi.emir_muhatabi = gelen_karakter;

					if(gelen_mesaj_yapisi.emir_muhatabi == yapi.emir_muhatabi)
					{
						gelen_mesaj[gelen_veri_adeti] = gelen_karakter;
					}
					else
					{
						olumsuz_durum++;

					}

					break;

				case 4:
					gelen_mesaj_yapisi.emir_adres[0] = gelen_karakter;

					if(gelen_mesaj_yapisi.emir_adres[0] == yapi.emir_adres[0])
					{
						gelen_mesaj[gelen_veri_adeti] = gelen_karakter;

					}
					else
					{
						olumsuz_durum++;
					}

					break;

				case 5:
					gelen_mesaj_yapisi.emir_adres[1] = gelen_karakter;

					if(gelen_mesaj_yapisi.emir_adres[1] == yapi.emir_adres[1])
					{
						gelen_mesaj[gelen_veri_adeti] = gelen_karakter;
					}
					else
					{
						olumsuz_durum++;
					}
					break;

				case 6:
					gelen_mesaj_yapisi.emir_adres[2] = gelen_karakter;

					if(gelen_mesaj_yapisi.emir_adres[2] == yapi.emir_adres[2])
					{
						gelen_mesaj[gelen_veri_adeti] = gelen_karakter;

					}
					else
					{
						olumsuz_durum++;
					}

					break;
				case 7:
					gelen_mesaj_yapisi.emir_adres[3] = gelen_karakter;

					if(gelen_mesaj_yapisi.emir_adres[3] == yapi.emir_adres[3])
					{
						gelen_mesaj[gelen_veri_adeti] = gelen_karakter;
					}
					else
					{
						olumsuz_durum++;
					}

					break;
				case 8:

					gelen_mesaj[gelen_veri_adeti] = gelen_karakter;
					gelen_mesaj_yapisi.emir_turu = gelen_karakter;

					if(gelen_mesaj_yapisi.emir_turu == yapi.emir_turu)
					{
						okuma_yazma = 1; // Okunacak.Değer Timer1 ARR register'ına  yazılacak
					}
					else
					{
						okuma_yazma = 0; //Yazılacak.Sayım kaçta ise  deger gönderilecek.
					}

					break;

				case 9:
					gelen_mesaj_yapisi.data_tipi = gelen_karakter;

					if(gelen_mesaj_yapisi.data_tipi == yapi.data_tipi)
					{
						gelen_mesaj[gelen_veri_adeti] = gelen_karakter;
					}
					else
					{
						olumsuz_durum++;
					}
					break;
				case 10:
					gelen_mesaj_yapisi.data_alani[0] = gelen_karakter;

					gelen_mesaj[gelen_veri_adeti] = gelen_karakter;

					break;
				case 11:
					gelen_mesaj_yapisi.data_alani[1] = gelen_karakter;
					gelen_mesaj[gelen_veri_adeti] = gelen_karakter;

					break;
				case 12:
					gelen_mesaj_yapisi.data_alani[2] = gelen_karakter;
					gelen_mesaj[gelen_veri_adeti] = gelen_karakter;

					break;
				case 13:
					gelen_mesaj_yapisi.data_alani[3] = gelen_karakter;
					gelen_mesaj[gelen_veri_adeti] = gelen_karakter;

					break;
				case 14:
					gelen_mesaj_yapisi.bitis = gelen_karakter;

					if(gelen_mesaj_yapisi.bitis == yapi.bitis)
					{
						gelen_mesaj[gelen_veri_adeti] = gelen_karakter;
					}
					else
					{
						olumsuz_durum++;
					}
					break;
			}
			gelen_veri_adeti++;
		}

		else
		{
			if(gelen_veri_adeti == 15  && olumsuz_durum == 0)
			{
				  hatasiz_mesaj++;
				  gelen_mesaj[16] = '\n';

			      binler = gelen_mesaj[10] - '0';

				  yuzler = gelen_mesaj[11] - '0';

				  onlar  = gelen_mesaj[12] - '0';

				  birler = gelen_mesaj[13] - '0';

				  if(okuma_yazma == 1)
				  {
					  toplam = binler*1000 + yuzler*100 + onlar*10 + birler;
					  TIM1->ARR = toplam; //Gelen değer ARR register'ına yazılıyor.
				  }
				  else if(okuma_yazma == 0)
				  {
					  //Timer1 in sayaç değeri gönderiliyor.

					  sayimin_gonderilcegi_an = TIM1->CNT;

					  //Timer1 in sayaç değeri basamaklara ayrılıyor.
					  birler = sayimin_gonderilcegi_an % 10;

					  onlar = (sayimin_gonderilcegi_an/ 10) % 10;
					  yuzler =  (sayimin_gonderilcegi_an/ 100) % 10;
					  binler = (sayimin_gonderilcegi_an / 1000) % 10;

					  gelen_mesaj[13] = birler + '0';
					  gelen_mesaj[12] = onlar  + '0';
					  gelen_mesaj[11] = yuzler + '0';
					  gelen_mesaj[10] = binler + '0';

					  yollanacak_mesaj(gelen_mesaj);  //Dışarıya mesaj gönderiliyor.
				  }
			}
			else
			{
				hatali_mesaj++;
				yollanacak_mesaj(hata_mesaji);
			}
			gelen_veri_adeti = 0;
			olumsuz_durum = 0;
		}
	}
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	yapi = yapi_tanimla();
	GPIO_Config();
	TIMER_Config();
	USART_Config();



  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  /* USER CODE BEGIN 2 */




  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  sayim_degeri = TIM1->CNT;  // STMStudio ortamında gözlemlemek için sayac degeri değişkende tutuluyor.

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
