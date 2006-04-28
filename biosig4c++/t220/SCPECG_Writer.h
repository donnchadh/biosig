/*
---------------------------------------------------------------------------
Copyright (C) 2005-2006  Franco Chiarugi
Developed at the Foundation for Research and Technology - Hellas, Heraklion, Crete

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
---------------------------------------------------------------------------
*/
/*
// SCPECG_Writer.h
*/

#ifndef VS_DEF
#include "biosig.h"
#else
#include "biosig_vs.h"
#endif

typedef struct SCPfile_head {
	uint16_t	crc;         /* 16 bit crc */
	uint32_t	len;         /* file lenght including this header */
} SCPFILE_HEAD;

typedef struct SCPsect_head {
	uint16_t	crc;         /* 16 bit crc */
	uint16_t	sectid;      /* section ID number */
	uint32_t	len;         /* section lenght including this section */
	uint8_t		sectver;     /* version number of the section */
	uint8_t		protver;     /* version number of the protocol */
	uint8_t		SCPres[6];   /* SCP reserved */
} SCPSECT_HEAD;

typedef struct SCPSECT0_row {
	uint16_t	sectid;
	uint32_t	sectlen;
	uint32_t	index;
} SCPSECT0_ROW;

typedef struct SCPSECT0 {
	SCPSECT_HEAD   shead;
	SCPSECT0_ROW   sect_0;  /* This section (pointers)       */
	SCPSECT0_ROW   sect_1;  /* Header info                   */
	SCPSECT0_ROW   sect_2;  /* Huffman tables                */
	SCPSECT0_ROW   sect_3;  /* ECG Lead Definition           */
	SCPSECT0_ROW   sect_4;  /* QRS Location                  */
	SCPSECT0_ROW   sect_5;  /* Reference Beat                */
	SCPSECT0_ROW   sect_6;  /* Lead (Signal) Data            */
	SCPSECT0_ROW   sect_7;  /* Global Measurements           */
	SCPSECT0_ROW   sect_8;  /* Textual Diagnosis             */
	SCPSECT0_ROW   sect_9;  /* Manufacturer Custom Diagnosis */
	SCPSECT0_ROW   sect_10; /* Lead Measurements             */
	SCPSECT0_ROW   sect_11; /* SCP Standard Diagnosis        */
} SCPSECT0;

typedef struct tag {
	uint8_t   id;
	uint16_t  len;
} TAG;
