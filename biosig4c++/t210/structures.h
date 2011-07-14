/*
---------------------------------------------------------------------------
Copyright (C) 2003  Eugenio Cervesato & Giorgio De Odorico.
Developed at the Associazione per la Ricerca in Cardiologia - Pordenone - Italy.

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
// structures.h          header file 

#ifndef __STRUCTURES_H__
#define __STRUCTURES_H__
#include <time.h>

#define int_S	int8_t
#define int_M	int16_t
#define int_L	int32_t
#define U_int_S	uint8_t
#define U_int_M	uint16_t
#define U_int_L	uint32_t
#define dec_S	float
#define dec_M	double
#define dec_L	long double
#define str	char

#define bool char
#define true 1
#define false 0
#define TRUE 1
#define FALSE 0

struct alfabetic
{
	U_int_M number;
	const char *sentence;
};

struct numeric
{
	U_int_M value;
	U_int_S unit;
} ;

struct section_header
{
	U_int_M CRC;
	U_int_M ID;
	U_int_L length;
	U_int_S version;
	U_int_S protocol_version;
	char *word;
};

struct file_header
{
	U_int_M CRC;
	U_int_L length;
};

struct pointer_section
{
	U_int_L index;
	U_int_M ID;
	U_int_L length;
};

struct device_info
{
	U_int_M institution_number;
	U_int_M department_number;
	U_int_M ID;
	U_int_S type;
	U_int_S manifacturer;
	char 	*model_description;
	U_int_S protocol_revision_number;
	U_int_S category;
	U_int_S language;
	U_int_S capability[4];
	U_int_S AC;
	char	*analysing_program_revision_number;
	char	*serial_number_device;
	char	*device_system_software;
	char	*device_SCP_implementation_software;
	char	*manifacturer_trade_name;
};

struct info_drug
{
	U_int_S table;
	U_int_S classes;
	U_int_S drug_code;
	U_int_M length; 
};	

struct Time_Zone
{
	int_M offset;
	U_int_M index;
	const char *description;
};

struct demographic
{
	char 	 *first_name;
	char 	 *last_name;
	char 	 *ID;
	char 	 *second_last_name;
	struct numeric  age;
        time_t   date_birth2;    // by E.C. feb 2006
	struct numeric  height;
	struct numeric  weight;
	U_int_S  sex;
	U_int_S  race;
	U_int_M  systolic_pressure;
	U_int_M  diastolic_pressure;
};

struct clinic
{
	U_int_M	number_drug;
	struct info_drug	*drug;
	char		*text_drug;

	U_int_M	number_diagnose;
	struct numeric	*diagnose;
	char		*text_diagnose;

	char		*referring_physician;
	char 		*latest_confirming_physician;
	char 		*technician_description;

	U_int_M	number_text;
	struct numeric	*free_text;
	char		*text_free_text;

	U_int_M	number_hystory;
	struct numeric	*medical_hystory;

	U_int_M	number_free_hystory;
	struct numeric	*free_medical_hystory;
	char		*text_free_medical_hystory;
};

struct descriptive
{
	struct device_info 	acquiring;
	struct device_info 	analyzing;
	char 		*acquiring_institution;
	char 		*analyzing_institution;
	char 		*acquiring_department;
	char 		*analyzing_department;
	char 		*room;
	U_int_S 	stat_code;
};

struct device
{
        time_t    date_acquisition2;       // by E.C. feb 2006
        time_t    time_acquisition2;       // by E.C. feb 2006
	U_int_M   baseline_filter;
	U_int_M   lowpass_filter;
	U_int_S   other_filter[4];
	char 	  *sequence_number;
	struct numeric   electrode_configuration;
	struct Time_Zone TZ;
};

struct table_H
{
	U_int_S bit_prefix;
	U_int_S bit_code;
	U_int_S TMS;
	int_M	 base_value;
	U_int_L base_code;
};

struct f_lead
{
	U_int_S number;
	bool 	 subtraction;
	bool 	 all_simultaneously;
	U_int_S number_simultaneously;
};

struct lead
{
	U_int_S ID;
	U_int_L start;
	U_int_L end;
};

struct Subtraction_Zone
{
	U_int_M beat_type;
	U_int_L SB;
	U_int_L fc;
	U_int_L SE;
};

struct Protected_Area
{
	U_int_L QB;
	U_int_L QE;
};

struct f_BdR0
{
	U_int_M length;
	U_int_M fcM;
	U_int_M AVM;
	U_int_M STM;
	U_int_M number_samples;
	U_int_S encoding;
};

struct f_Res
{
	U_int_M AVM;
	U_int_M STM;
	U_int_M number;
	U_int_M number_samples;
	U_int_S encoding;
	bool bimodal;
	U_int_S decimation_factor;
};

struct spike
{
	U_int_M time;
	int_M amplitude;
	U_int_S type;
	U_int_S source;
	U_int_S index;
	U_int_M pulse_width;
};

struct global_measurement
{
	U_int_S number;
	U_int_M number_QRS;
	U_int_S number_spike;
	U_int_M average_RR;
	U_int_M average_PP;
	U_int_M ventricular_rate;
	U_int_M atrial_rate;
	U_int_M QT_corrected;
	U_int_S formula_type;
	U_int_M number_tag;
};

struct additional_measurement
{
	U_int_S ID;
	U_int_S byte[5];
};

struct BdR_measurement
{
	U_int_M P_onset;
	U_int_M P_offset;
	U_int_M QRS_onset;
	U_int_M QRS_offset;
	U_int_M T_offset;
	int_M P_axis;
	int_M QRS_axis;
	int_M T_axis;
};

struct info
{
	U_int_S type;
	char *date;
	char *time;
	U_int_S number;
};

struct header_lead_measurement
{
	U_int_M number_lead;
	U_int_M number_lead_measurement;
};

struct lead_measurement_block
{
	U_int_M ID;
	int_M P_duration;
	int_M PR_interval;
	int_M QRS_duration;
	int_M QT_interval;
	int_M Q_duration;
	int_M R_duration;
	int_M S_duration;
	int_M R1_duration;
	int_M S1_duration;
	int_M Q_amplitude;
	int_M R_amplitude;
	int_M S_amplitude;
	int_M R1_amplitude;
	int_M S1_amplitude;
	int_M J_point_amplitude;
	int_M Pp_amplitude;
	int_M Pm_amplitude;
	int_M Tp_amplitude;
	int_M Tm_amplitude;
	int_M ST_slope;
	int_M P_morphology;
	int_M T_morphology;
	int_M iso_electric_segment_onset_QRS;
	int_M iso_electric_segment_offset_QRS;
	int_M intrinsicoid_deflection;
	U_int_M quality_recording[8];
	int_M ST_amplitude_Jplus20;
	int_M ST_amplitude_Jplus60;
	int_M ST_amplitude_Jplus80;
	int_M ST_amplitude_JplusRR16;
	int_M ST_amplitude_JplusRR8;
};

struct statement_coded
{
	U_int_S sequence_number;
	U_int_M length;
	U_int_S type;
	U_int_M number_field;
};

//_____________________________________
//structs for sections: 2, 3, 4, 5, 6
//_____________________________________
struct DATA_DECODE
{
	struct table_H *t_Huffman;
	U_int_M *flag_Huffman;

	struct lead *data_lead;
	struct f_lead flag_lead;

	struct Protected_Area *data_protected;
	struct Subtraction_Zone *data_subtraction;

	struct f_BdR0 flag_BdR0;
	U_int_M *length_BdR0;
	U_int_S *samples_BdR0;
	int_L *Median;

	struct f_Res flag_Res;
	U_int_M *length_Res;
	U_int_S *samples_Res;
	int_L *Residual;

	int_L *Reconstructed;
};

struct TREE_NODE
//struttura di un nodo dell'albero
{
	struct TREE_NODE *next_0;
	struct TREE_NODE *next_1;
	int_M row;
};

//_____________________________________
//structs for sections: 7, 10
//_____________________________________
struct DATA_RECORD
{
	struct global_measurement data_global;
	struct spike *data_spike;
	U_int_S *type_BdR;
	struct BdR_measurement *data_BdR;
	struct additional_measurement *data_additional;

	struct header_lead_measurement header_lead;
	struct lead_measurement_block *lead_block;
};

//_____________________________________
//structs for sections: 1, 8, 11
//_____________________________________
struct DATA_INFO
{
	struct demographic ana;
	struct clinic cli;
	struct descriptive des;
	struct device dev;

	struct info flag_report;
	struct numeric *text_dim;
	char *text_report;

	struct info flag_statement;
	struct statement_coded *data_statement;
	char *text_statement;
};

#endif /*__STRUCTURES_H__*/
//_____________________________________
