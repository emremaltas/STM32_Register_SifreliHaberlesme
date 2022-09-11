/*
 * mesaj_degiskenleri.h
 *
 *  Created on: 26 Haz 2022
 *      Author: User
 */

#ifndef MESAJ_YAPISI_MESAJ_DEGISKENLERI_H_
#define MESAJ_YAPISI_MESAJ_DEGISKENLERI_H_

#include "stm32f4xx_hal.h"

typedef struct
{
	unsigned char  baslangic;
	uint16_t gelen_emir_numarasi[2];
	uint8_t emir_muhatabi;
	uint32_t emir_adres[4];
	uint8_t emir_turu;
	unsigned char data_tipi;
	uint8_t data_alani[4];
	uint16_t bitis;

}MesajYapisiTypeDef;

MesajYapisiTypeDef a;

MesajYapisiTypeDef yapi_tanimla()
{
	a.baslangic = '£';   //başlama karakteri

	a.gelen_emir_numarasi[0] = 50;  // emir no 1
	a.gelen_emir_numarasi[1] = 50;  // emir no 2

	a.emir_muhatabi = 51;           //emir muhatabı

	a.emir_adres[0] = 48;		   //adres 1. indisi
	a.emir_adres[1] = 48;		   //adres 2. indisi
	a.emir_adres[2] = 49;		   //adres 3. indisi
	a.emir_adres[3] = 56;		   //adres 4. indisi

	a.emir_turu = 56;			   //8 harici her şey okunacak

	a.data_tipi = 'i';             //data tipi int olacak

	a.data_alani[0] = 0;		   //Verinin 1. değeri
	a.data_alani[1] = 0;		   //Verinin 2. değeri
	a.data_alani[2] = 0;		   //Verinin 3. değeri
	a.data_alani[3] = 0;		   //Verinin 4. değeri

	a.bitis = '%';     //bitiş karakteri

 return a;
}



#endif /* MESAJ_YAPISI_MESAJ_DEGISKENLERI_H_ */
