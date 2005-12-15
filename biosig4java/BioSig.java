/*
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.


    $Id: BioSig.java,v 1.2 2005-12-15 15:16:29 schloegl Exp $
    Copyright (C) 2005 Alois Schloegl <a.schloegl@ieee.org>
    This function is part of the "BioSig for Java" repository 
    (biosig4java) at http://biosig.sf.net/ 

 */


import java.io.*;

public class BioSig {
    
    static class HDRTYPE {
         int 	TYPE;
         float 	VERSION; 
         int	HeadLen; 

         String FileName;
         FileInputStream 	fileIn;
         FileOutputStream 	fileOut;
         int		FileOpen; 	//0: closed, 1: read, 2: write 
  	 int		FilePos; 	// position of file handle        

         double		T0; 	// start date and time - format might change
         int		NS; 	// number of channels
         int 		SPR;	// samples per record
         int		NRec; 	// number of Records
      	 int[]		Duration = new int[2];
      	 double		SampleRate; 
      	 
         double		patientBirthday; 	// date and time - format might change
	 String		patientName; 
	 String		patientID; 
	 float		patientWeight;
	 float		patientHeight;
	 float[]	HeadSize;
	 int		patientSex; 
	 int		patientHandedness; 
	 	
         boolean	flagOverflowdetection; 
         boolean	flagUCAL; 	// 0: calibrated, 1: un-calibrated

  	 CHANNEL[]	channel;	

  	 double[][]	data; 
  	 int[][]	rawdata; 
	
	 EVENT[]	event; 	
    }


    static class CHANNEL {
	 boolean	OnOff; 		// 
      	 String 	Label; 
      	 String		Transducer;

	 String		PhysDim;	// physical dimension
	 int		PhysDimCode;	// code for physical dimension
	//char* 	PreFilt;	// pre-filtering

	float 		LowPass;	// lowpass filter
	float 		HighPass;	// high pass
	float 		Notch;		// notch filter
	float[] 	XYZ = new float[3];		// electrode position
	float 		Impedance;   	// in Ohm
	
	double 		PhysMin;	// physical minimum
	double 		PhysMax;	// physical maximum
	double 		DigMin;		// digital minimum
	double	 	DigMax;		// digital maximum

	int 		GDFTYP;		// data type
	int 		SPR;		// samples per record (block)
	
	double		Cal;		// gain factor 
	double		Off;		// bias       	 
    }

    static class EVENT {
	 int		POS;	
	 int		TYP;	
	 int		CHN;	
	 int		DUR;	
    }
     
    static HDRTYPE getfiletype (HDRTYPE HDR) {
  	// identification of file format 
	byte[] b = new byte[256];
	String[] MagicKeys = {"0       ","GDF ","\255BIOSEMI", "SCP","XML"};
		

	try {
		HDR.fileIn.read(b); 
	//	    ss = string(b); 
/*
	if (compare(b{0:7},{'0',0,0,0,0,0,0,0 }) == null) {
		System.out.println("EDF");
	}
	    else if (compare(b,{'G','D','F',' ' }) == null) { 
		    System.out.println("GDF");
	    }    
	    else if (compare(b,"BIOSEMI") == null) { 
		    System.out.println("BDF");
	    }    
	    else
		    System.out.println("unknown");
	// do file type check 				
*/

			System.out.write(b);

		} 
		catch (Exception e) {
			System.out.println("error " + e);
		}	
		return(HDR);
  	}
  

    	public static HDRTYPE sopen (String[] args, String PERMISSION) {
		String FileName = args[0];

  		HDRTYPE HDR = new HDRTYPE(); 
  		HDR.FileName = FileName; 
	
		if (PERMISSION.compareTo("r")==0) { 
		try {
	  		HDR.fileIn = new FileInputStream (HDR.FileName);
  			HDR.FileOpen = 1;
			HDR = getfiletype (HDR); 	
			
			
/********* add your sopenReadSCP here  **********/ 

/********* add your sopenReadHL7aECG here   **********/ 
			
		}
		catch (Exception e) {
			System.out.println("file " + HDR.FileName + " not found");
		}	
		}
		
		else if (PERMISSION.compareTo("w")==0 ) { 
		try {
	  		HDR.fileOut = new FileOutputStream (HDR.FileName);
  			HDR.FileOpen = 2;

/********* add your sopenWriteSCP here  **********/ 

/********* add your sopenWriteHL7aECG here  *********/
 
		}
		catch (Exception e) {
			System.out.println("file " + HDR.FileName + " not found");
		}	
		}
		

/*
		if compare(PERMISSION,'r') and (compare(HDR.TYPE == SCP)1=null) {
			sopenScpRead(); 			
		}
		elseif compare(PERMISSION,'w') and (filetype == SCP) {
			sopenScpWrite(); 			
		}
		elseif compare(PERMISSION,'r') and (filetype == aECG) {
			sopenHL7aECGRead(); 			
		}
		elseif compare(PERMISSION,'w') and (filetype == aECG) {
			sopenHL7aECGwrite(); 			
		}
*/		
		return(HDR); 
		
	}
  
	public static HDRTYPE sread (HDRTYPE HDR, int length, int startPosition) {
		// introduced to have a similar structure to Matlab and C/C++
		/* 
		for SCP and XML data this might be empty, because data has been read in SOPEN
		*/ 
		return(HDR); 
	}
  

	public static HDRTYPE swrite (HDRTYPE HDR, int[][] data) {
		// introduced to have a similar structure to Matlab and C/C++
		/* 
		the imput argument for SCP and XML data this might be empty, because data has been read in SOPEN
		*/ 
		return(HDR); 
	}

  
	public static HDRTYPE sclose (HDRTYPE HDR) {
		
		try {
			if (HDR.FileOpen==1) HDR.fileIn.close();
			else if (HDR.FileOpen==2) HDR.fileOut.close();
			HDR.FileOpen=0; 
		}
		catch (Exception e) {
		}
		return(HDR); 
	}

  
	public static boolean seof (HDRTYPE HDR) {
		// introduced to have a similar structure to Matlab and C/C++
		
		return(HDR.FilePos>=HDR.NRec); 
	}

  
	public static void main (String[] args) {
		sclose(sopen(args,"r")); 
		System.out.println(args[0]);
	}
}


