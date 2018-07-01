#ifndef SAKURA_HEADERMAKE_2034D8F5_AE65_408D_9F53_D3DEA240C67BI_H_
#define SAKURA_HEADERMAKE_2034D8F5_AE65_408D_9F53_D3DEA240C67BI_H_

enum EFunctionCode{
	#define DEF_COMMAND(   id, val )													id = val,
	#define DEF_MACROFUNC( id, val, name, param0, param1, param2, param3, ret, ext )	id = val,
	#define DEF_MACROCMD(  id, val, name, param0, param1, param2, param3, ret, ext )	id = val,
	#define DEF_MACROFUNC2(id,      name, param0, param1, param2, param3, ret, ext )
	#define DEF_MACROCMD2( id,      name, param0, param1, param2, param3, ret, ext )
	
	#include "Funccode_x.h"
};

#endif /* SAKURA_HEADERMAKE_2034D8F5_AE65_408D_9F53_D3DEA240C67BI_H_ */
/*[EOF]*/
