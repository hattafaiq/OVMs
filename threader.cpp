#include "threader.h"

extern struct d_global global;

threader::threader()
{

}

void threader::run()
{
base->save_database_waveform_harian(global.db, bb1,num,fmax,ref_rpm,safe_to_save_ch);
}
