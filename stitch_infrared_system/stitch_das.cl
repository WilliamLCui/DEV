
enum
{
    INTER_BITS = 5,
    INTER_TAB_SIZE = 1 << INTER_BITS,
    INTER_TAB_SIZE2 = INTER_TAB_SIZE * INTER_TAB_SIZE
};

#define INTER_LINEAR  //INTER_NEAREST INTER_LINEAR
#define BORDER_REFLECT_101

#ifdef BORDER_REPLICATE
#define EXTRAPOLATE(v2) \
    { \
        v2 = max(min(v2, (int2)(inw - 1, inh - 1)), (int2)(0)); \
    }
#elif defined BORDER_WRAP
#define EXTRAPOLATE(v2) \
    { \
        if (v2.x < 0) \
            v2.x -= ((v2.x - inw + 1) / inw) * inw; \
        if (v2.x >= inw) \
            v2.x %= inw; \
        \
        if (v2.y < 0) \
            v2.y -= ((v2.y - inh + 1) / inh) * inh; \
        if( v2.y >= inh ) \
            v2.y %= inh; \
    }
#elif defined(BORDER_REFLECT) || defined(BORDER_REFLECT_101)
#ifdef BORDER_REFLECT
#define DELTA int delta = 0
#else
#define DELTA int delta = 1
#endif
#define EXTRAPOLATE(v2) \
    { \
        DELTA; \
        if (inw == 1) \
            v2.x = 0; \
        else \
            do \
            { \
                if( v2.x < 0 ) \
                    v2.x = -v2.x - 1 + delta; \
                else \
                    v2.x = inw - 1 - (v2.x - inw) - delta; \
            } \
            while (v2.x >= inw || v2.x < 0); \
        \
        if (inh == 1) \
            v2.y = 0; \
        else \
            do \
            { \
                if( v2.y < 0 ) \
                    v2.y = -v2.y - 1 + delta; \
                else \
                    v2.y = inh - 1 - (v2.y - inh) - delta; \
            } \
            while (v2.y >= inh || v2.y < 0); \
    }
#else
#error No extrapolation method
#endif

#define NEED_EXTRAPOLATION(gx, gy) (gx >= inw || gy >= inh || gx < 0 || gy < 0)



#define inw 640
#define inh 512

__kernel void stitch_6_das_assign(
__global uchar* inputimg0,__global int* map0,int data_size0, __global float* weight0,int weight_size0,
__global uchar* inputimg1,__global int* map1,int data_size1,__global float* weight1,int weight_size1,
__global uchar* inputimg2,__global int* map2,int data_size2,__global float* weight2,int weight_size2,
__global uchar* inputimg3,__global int* map3,int data_size3,__global float* weight3,int weight_size3,
__global uchar* inputimg4,__global int* map4,int data_size4,__global float* weight4,int weight_size4,
__global uchar* inputimg5,__global int* map5,int data_size5,__global float* weight5,int weight_size5,
__global uchar* outputimg)
{
	int idx, src_index, src1_index, dst_index;
	float weight=1.0f;

	for(idx=get_global_id(0);idx<1970433; idx+=get_global_size(0))
	{
		outputimg[idx] = 0;
	}
	barrier(CLK_GLOBAL_MEM_FENCE);

	//------------map  weight  mask----------------------//
	for(idx=get_global_id(0);idx<data_size0; idx+=get_global_size(0))
	{
		src_index = map0[idx*2];
		dst_index = map0[idx*2+1];
		weight = weight0[idx];
		outputimg[dst_index] += inputimg0[src_index] * weight;
	}
	barrier(CLK_GLOBAL_MEM_FENCE);

	for(idx=get_global_id(0);idx<data_size2; idx+=get_global_size(0))
	{
		src_index = map2[idx*2];
		dst_index = map2[idx*2+1];
		weight = weight2[idx];
		outputimg[dst_index] += inputimg2[src_index] * weight;
	}
	barrier(CLK_GLOBAL_MEM_FENCE);

	for(idx=get_global_id(0);idx<data_size4; idx+=get_global_size(0))
	{
		src_index = map4[idx*2];
		dst_index = map4[idx*2+1];
		weight = weight4[idx];
		outputimg[dst_index] += inputimg4[src_index] * weight;
	}
	barrier(CLK_GLOBAL_MEM_FENCE);

	for(idx=get_global_id(0);idx<data_size1; idx+=get_global_size(0))	
	{
		src_index = map1[idx*2];
		dst_index = map1[idx*2+1];
		weight = weight1[idx];
		outputimg[dst_index] += inputimg1[src_index] * weight;
	}
	barrier(CLK_GLOBAL_MEM_FENCE);
	for(idx=get_global_id(0);idx<data_size5; idx+=get_global_size(0))	
	{					
		src_index = map5[idx*2];		
		dst_index = map5[idx*2+1];
		weight = weight5[idx];
		outputimg[dst_index] += inputimg5[src_index] * weight;
	}
	barrier(CLK_GLOBAL_MEM_FENCE);
	
	for(idx=get_global_id(0);idx<data_size3; idx+=get_global_size(0))	
	{					
		src_index = map3[idx*2];
		dst_index = map3[idx*2+1];
		weight = weight3[idx];
		outputimg[dst_index] += inputimg3[src_index] * weight;
	}
	barrier(CLK_GLOBAL_MEM_FENCE);

}
















