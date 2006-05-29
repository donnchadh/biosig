
/*
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.


    $Id: BioSig.java,v 1.5 2006-05-29 19:46:29 schloegl Exp $
    Copyright (C) 2005,2006 Alois Schloegl <a.schloegl@ieee.org>
    This function is part of the "BioSig for Java" repository
    (biosig4java) at http://biosig.sf.net/

 */



/*
TODO
        conversion of LE to BE. support of byte swaping
*/
package jaecgreader;

//~--- JDK imports ------------------------------------------------------------

import java.io.*;

//~--- classes ----------------------------------------------------------------

public class BioSig {
    static HDRTYPE getfiletype(HDRTYPE HDR) {

        // identification of file format
        byte[] b = new byte[256];

        try {
            HDR.fileIn.read(b);
            HDR.VERSION = -1;

            if (new String(b, 0, 8).equals("0       ")) {
                HDR.TYPE    = BioSig.FileFormat.EDF;
                HDR.VERSION = 0;
            } else if (new String(b, 0, 3).equals("GDF")) {
                HDR.TYPE    = BioSig.FileFormat.GDF;
                HDR.VERSION = Float.valueOf(new String(b, 3, 5)).floatValue();
            } else if (new String(b, 1, 7).equals("BIOSEMI") & (b[0] == -1)) {
                HDR.TYPE    = BioSig.FileFormat.BDF;
                HDR.VERSION = -1;
            } else if (new String(b, 16, 6).equals("SCPECG")) {
                HDR.TYPE = BioSig.FileFormat.SCP;
            } else if (new String(b, 0, 38).equals(
                    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>")) {
                HDR.TYPE = BioSig.FileFormat.XML_UTF8;
            } else if (new String(b, 0, 5).equals("<?xml")) {
                HDR.TYPE = BioSig.FileFormat.XML;
            } else {}

//          System.out.write(b);
        } catch (Exception e) {
            System.out.println("error " + e);
        }

        return (HDR);
    }

    public static void main(String[] args) {
        sclose(sopen(args, "r"));
        System.out.println(args[0]);
    }

    public static HDRTYPE sclose(HDRTYPE HDR) {
        try {
            if (HDR.FileOpen == 1) {
                HDR.fileIn.close();
            } else if (HDR.FileOpen == 2) {
                HDR.fileOut.close();
            }

            HDR.FileOpen = 0;
        } catch (Exception e) {}

        return (HDR);
    }

    public static boolean seof(HDRTYPE HDR) {

        // introduced to have a similar structure to Matlab and C/C++
        return (HDR.FilePos >= HDR.NRec);
    }

    public static HDRTYPE sopen(String[] args, String PERMISSION) {
        String  FileName = args[0];
        HDRTYPE HDR      = new HDRTYPE();

        HDR.FileName = FileName;

        byte[] b = new byte[256];
        int    k;

        if (PERMISSION.compareTo("r") == 0) {
            try {
                HDR.fileIn = new RandomAccessFile(HDR.FileName, "r");

//              HDR.fileIn = new FileInputStream (HDR.FileName);
                HDR.fileIn.read(b);
                HDR.FileOpen = 1;
                HDR.VERSION  = -1;

                if (new String(b, 0, 8).equals("0       ")) {
                    HDR.TYPE    = BioSig.FileFormat.EDF;
                    HDR.VERSION = 0;
                } else if (new String(b, 0, 3).equals("GDF")) {
                    HDR.TYPE    = BioSig.FileFormat.GDF;
                    HDR.VERSION = Float.valueOf(new String(b, 3,
                            5)).floatValue();
                } else if (new String(b, 1, 7).equals("BIOSEMI")
                           & (b[0] == -1)) {
                    HDR.TYPE    = BioSig.FileFormat.BDF;
                    HDR.VERSION = -1;
                } else if (new String(b, 16, 6).equals("SCPECG")) {
                    HDR.TYPE = BioSig.FileFormat.SCP;
                } else if (new String(b, 0, 38).equals(
                        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>")) {
                    HDR.TYPE = BioSig.FileFormat.XML_UTF8;
                } else if (new String(b, 0, 5).equals("<?xml")) {
                    HDR.TYPE = BioSig.FileFormat.XML;
                } else {}

//              System.out.write(b);
                switch (HDR.TYPE) {
                case BioSig.FileFormat.BDF :
                    System.out.println("BDF");

                    break;

                case BioSig.FileFormat.EDF :
                    System.out.println("EDF");

                    break;

                case BioSig.FileFormat.GDF :
                    HDR.fileIn.seek(236);
                    HDR.NRec = HDR.fileIn.readLong();
                    HDR.fileIn.seek(252);
                    HDR.NS = HDR.fileIn.readInt();
                    System.out.println(HDR.NS);
                    System.out.println(HDR.NRec);

                    break;

                case BioSig.FileFormat.SCP :
                    System.out.println("SCP");

                    break;

                case BioSig.FileFormat.XML :
                    System.out.println("XML");

                    break;

                case BioSig.FileFormat.XML_UTF8 :
                    System.out.println("XML-UTF8");

                    break;

                default :
                    System.out.println("unknown fileformat");

                    break;
                }

/********* add your sopenReadSCP here  **********/

/********* add your sopenReadHL7aECG here   **********/
            } catch (Exception e) {
                System.out.println("file " + HDR.FileName + " not found");
            }
        } else if (PERMISSION.compareTo("w") == 0) {
            try {
                HDR.fileOut  = new RandomAccessFile(HDR.FileName, "w");
                HDR.FileOpen = 2;

/********* add your sopenWriteSCP here  **********/

/********* add your sopenWriteHL7aECG here  *********/
            } catch (Exception e) {
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
        return (HDR);
    }

    public static HDRTYPE sread(HDRTYPE HDR, int length, int startPosition) {

        // introduced to have a similar structure to Matlab and C/C++

        /*
         * for SCP and XML data this might be empty, because data has been read in SOPEN
         */
        return (HDR);
    }

    public static HDRTYPE swrite(HDRTYPE HDR, int[][] data) {

        // introduced to have a similar structure to Matlab and C/C++

        /*
         * the imput argument for SCP and XML data this might be empty, because data has been read in SOPEN
         */
        return (HDR);
    }

    //~--- inner classes ------------------------------------------------------

    static class CHANNEL {
        float[] XYZ = new float[3];    // electrode position
        double  Cal;                   // gain factor
        double  DigMax;                // digital maximum
        double  DigMin;                // digital minimum
        int     GDFTYP;                // data type
        float   HighPass;              // high pass
        float   Impedance;             // in Ohm
        String  Label;
        float   LowPass;               // lowpass filter
        float   Notch;                 // notch filter
        double  Off;                   // bias
        boolean OnOff;                 //
        String  PhysDim;               // physical dimension
        int     PhysDimCode;           // code for physical dimension

        // char*         PreFilt;        // pre-filtering

        double  PhysMax;               // physical maximum
        double  PhysMin;               // physical minimum
        int     SPR;                   // samples per record (block)
        String  Transducer;
    }


    static class EVENT {
        int CHN;
        int DUR;
        int POS;
        int TYP;
    }


    public class FileFormat {
        public static final int BDF      = -1;
        public static final int EDF      = 0;
        public static final int GDF      = 1;
        public static final int SCP      = 2;
        public static final int XML      = 3;
        public static final int XML_UTF8 = 88;
    }


//  static enum Days { SUNDAY, MONDAY, TUESDAY, WEDNESDAY, THURSDAY,  FRIDAY, SATURDAY };
//e num FileFormat {ACQ, BKR, BDF, CFWB, CNT, DEMG, EDF, EVENT, FLAC, GDF, MFER, NEX1, PLEXON, SCP_ECG, HL7aECG, XML}; 
    static class HDRTYPE {
        int[]      Duration = new int[2];
        String     FileName;
        int        FileOpen;    // 0: closed, 1: read, 2: write
        int        FilePos;     // position of file handle
        int        HeadLen;
        float[]    HeadSize;
        long       NRec;        // number of Records
        int        NS;          // number of channels
        long       SPR;         // samples per record
        double     SampleRate;
        double     T0;          // start date and time - format might change
        int        TYPE;        // of type BioSig.FileFormat
        float      VERSION;
        CHANNEL[]  channel;
        double[][] data;
        EVENT[]    event;

//      FileInputStream         fileIn;
//      FileOutputStream        fileOut;
        RandomAccessFile fileIn;
        RandomAccessFile fileOut;
        boolean          flagOverflowdetection;
        boolean          flagUCAL;    // 0: calibrated, 1: un-calibrated
        double           patientBirthday;    // date and time - format might change
        int              patientHandedness;
        float            patientHeight;
        String           patientID;
        String           patientName;
        int              patientSex;
        float            patientWeight;
        int[][]          rawdata;
    }
}
