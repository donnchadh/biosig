//
// UFPE_BioSig.c
// Biosig interface in IGOR
//

#include "../biosig.h"

#include "XOPStandardHeaders.h"    // Include ANSI headers, Mac headers, IgorXOP.h, XOP.h and XOPSupport.h

static HDRTYPE *hdr = NULL;

int xopSLOAD( void * );  // ...Handle FName; Handle sRes; }* p)
/*
int UFPE_BioSigOpen(		void * );  // ...Handle FName; Handle sRes; }* p)
int UFPE_BioSigClose(		void * );  // double hnd;    Handle sRes; }* p)
int UFPE_BioSigRead(		void * );  // double hnd;    Handle sRes; }* p)
*/

/////////////////////////////////////////////////////////////////////////////////////////

// SAME ORDER HERE IN  '(*sFunc[])' AND IN UF_UtilsWinCustom.RC
/*
int (*)sFunc(void)[] =
  //  Der Name der      Direct Call method
  //  Funktion          or Message method	   Used that often
{
   { UFPE_BioSigOpen				},   
   { UFPE_BioSigClose					},   
//   { UFPE_BioSigRead					},   
   {    NULL       }  // Endemarkierung
};
*/
////////////////////////////////////////////////////////////////////////


typedef struct { char *FileName; waveHndl data; double res; HDRTYPE* hdr;} inSLOAD_t;
typedef struct { char *FileName; double res; } inSOpen_t;
typedef struct { double res; } inSClose_t;

int sload( inSLOAD_t* p)
// IGOR wrapper for Biosig.SLoad()
{
	// read header
	hdr = sopen( p->FileName, "r", hdr ); 
	p->res = serror();
	if (p->res) return(0);

	// read data
	biosig_data_type* data = NULL;
	sread(data, 0, -1, hdr);
	p->res = serror();
	if (p->res) return(0);

	// close file
	sclose(hdr);
	p->res = serror();
	return(0);
} 


int UFPE_BioSigOpen( inSOpen_t* p)
// IGOR wrapper for Biosig.SOpen()
{

   int    hState;
      hdr = sopen( p->FileName, "r", hdr ); 
      p->res = (double)B4C_ERRNUM;				// 
      if ( B4C_ERRNUM )
      {
      	 char buf[500];
         sprintf( buf, "\t\tUFPE_BioSigOpen...receives '%s' : %s,  returns code=%d \r",
                          p->FileName, B4C_ERRMSG, B4C_ERRNUM ); 
         XOPNotice( buf ); 
		 B4C_ERRNUM = B4C_NO_ERROR;
	  }
  p->res = 0; 
  return 0;								// Returning 0 prevents IGOR from doing anything with the error (no error box, no debugger)
}

							   
int UFPE_BioSigClose( inSClose_t *p)
{
   short     code    = sclose( hdr );
   p->res = serror();                        // ..we return the error code to the calling function. After having done all that...
   return 0;                             // ..we don't want IGOR to do anything with the error (no error box, no debugger)
}


/*
int UFPE_BioSigRead( struct { double ErrMode; waveHndl wDataADS; double Bytes; double StartOffset; double DataSection; double hnd; double res; }* p)
// IGOR wrappper for CFS ReadData()
// Difference to CFS: as IGOR doesn't know pointers the CFS buffer is passed as a wave   
{
   short     hnd         = (short)p->hnd;
   WORD      DataSection = (WORD)p->DataSection;
   long      StartOffset = (long)p->StartOffset;
   WORD      Bytes       = (WORD)p->Bytes;
   short     code;
   void *Raw;

	// DebugPrintWaveProperties( "UFPE_CfsReadData    ", p->wDataADS ); 	// 050128

	if ( p->wDataADS == NIL )                // check if wave handle is valid
   {
		SetNaN64( &p->res );			           // return NaN if wave is not valid   
		return( NON_EXISTENT_WAVE );
	}
 	if (   WaveType(p->wDataADS) != NT_I16 ) // check wave's numeric type  
   {
  	   SetNaN64( &p->res );				        // return NaN if wave is not 2Byte int
		return( IS_NOT_2BYTE_INT_WAVE );
	}

   Raw  = WaveData( p->wDataADS ); //  char pointer to IGOR wave data 

   code = ReadData( hnd, DataSection, StartOffset, Bytes, Raw );
   if ( code < 0 )
      PrintFileError( (int)p->ErrMode );

   p->res = (double)code;              // We handle the errors here, but we also return the error code to the calling function
   return 0;                           // ..we don't want IGOR to do anything with the error (no error box, no debugger)
}

// END   OF BIOSIG INTERFACE
/////////////////////////////////////////////////////////////////////////////////////////

*/
