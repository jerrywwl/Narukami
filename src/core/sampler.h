/*
MIT License

Copyright (c) 2019 ZhuQian

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#pragma once
#include "core/narukami.h"
#include "core/affine.h"
#include "core/rng.h"
#include "core/lowdiscrepancy.h"
#include <vector>
NARUKAMI_BEGIN
    class Sampler{
        private:
            uint32_t _current_sample_index;
            uint32_t _spp;
            const uint32_t _max_dim;
            uint32_t _sample_1d_offset;
            uint32_t _sample_2d_offset;
            std::vector<uint32_t> _scramble_1d;
            std::vector<uint32_t> _scramble_2d;
            
            std::vector<uint32_t> _scramble_1d_array;
            std::vector<uint32_t> _sample_1d_array_index;
            uint32_t _array_1d_offset;

            std::vector<uint32_t> _scramble_2d_array;
            std::vector<uint32_t> _sample_2d_array_index;
            uint32_t _array_2d_offset;

            Point2i  _current_pixel;
            uint32_t _1d_array_count=0;
            uint32_t _2d_array_count=0;
            
            RNG _rng;
        public:
            Sampler(const uint32_t spp,const uint32_t max_dim=5):_spp(spp),_max_dim(max_dim){
                _scramble_1d=std::vector<uint32_t>(_max_dim);
                _scramble_2d=std::vector<uint32_t>(_max_dim*2);
            }

            void start_pixel(const Point2i& p){
                _current_pixel=p;
                _current_sample_index=0;
                _sample_1d_offset = 0;
                _sample_2d_offset = 0;
                //generate all scramble number
                for (size_t i = 0; i < _max_dim; i++)
                {
                   _scramble_1d[i]=_rng.next_uint32();
                   _scramble_2d[i*2]=_rng.next_uint32();
                   _scramble_2d[i*2+1]=_rng.next_uint32();
                }

                _array_1d_offset = 0;
                for (size_t i = 0; i < _scramble_1d_array.size(); i++)
                {
                    _scramble_1d_array[i]=_rng.next_uint32();
                    _sample_1d_array_index[i] = 0;
                }

                _array_2d_offset = 0;
                for (size_t i = 0; i < _scramble_2d_array.size(); i++)
                {
                    _scramble_2d_array[i]=_rng.next_uint32();
                }
                for (size_t i = 0; i < _sample_2d_array_index.size(); i++)
                {
                   _sample_2d_array_index[i] = 0;
                }
                
                
            }

            bool start_next_sample(){
                _current_sample_index++;
                if (EXPECT_NOT_TAKEN(_current_sample_index>=_spp)){
                    return false;
                }
                _sample_1d_offset = 0;
                _sample_2d_offset = 0;
                return true;
            }


            Point2f get_2D(){
                if(EXPECT_TAKEN(_sample_2d_offset<_max_dim)){
                    auto sample= sample_scrambled_gray_code_sobol02(_current_sample_index,&_scramble_2d[_sample_2d_offset*2],&_scramble_2d[_sample_2d_offset*2+1]);
                    _sample_2d_offset++;
                    return sample;
                }
                else{
                    auto sample= Point2f(_rng.next_float(),_rng.next_float());
                    _sample_2d_offset++;
                    return sample;
                }
                 
            }

            float get_1D(){
                 if(EXPECT_TAKEN(_sample_1d_offset<_max_dim)){
                     auto sample=sample_scrambled_gray_code_van_der_corput(_current_sample_index,&_scramble_1d[_sample_1d_offset]);
                     _sample_1d_offset++;
                     return sample;
                 }
                 else{
                     _sample_1d_offset++;
                     return _rng.next_float();
                 }
            }

            void request_1d_array(const uint32_t request_count=1){
                _1d_array_count+=request_count;
            }

             void request_2d_array(const uint32_t request_count=1){
                _2d_array_count+=request_count;
            }

            void commit(){
                _scramble_1d_array = std::vector<uint32_t>(_1d_array_count);
                _sample_1d_array_index = std::vector<uint32_t>(_1d_array_count);

                _scramble_2d_array = std::vector<uint32_t>(_2d_array_count*2);
                _sample_2d_array_index = std::vector<uint32_t>(_2d_array_count);
            } 

            bool get_1D_array(const uint32_t num,float* array){
                if (_array_1d_offset==_1d_array_count){
                        return false;
                }
                for (size_t i = 0; i < num; i++)
                {
                    array[i]=sample_scrambled_gray_code_van_der_corput(_sample_1d_array_index[_array_1d_offset]++,&_scramble_1d_array[_array_1d_offset]);
                }
                _array_1d_offset++;
                return true;
            }

            bool get_2D_array(const uint32_t num,Point2f* array){
                if (_array_2d_offset==_2d_array_count){
                        return false;
                }
                for (size_t i = 0; i < num; i++)
                {
                    array[i]=sample_scrambled_gray_code_sobol02(_sample_2d_array_index[_array_2d_offset]++,&_scramble_2d_array[i*_array_2d_offset],&_scramble_2d_array[i*_array_2d_offset+1]);
                }
                _array_2d_offset++;
                return true;
            }

            void set_sample_index(const uint32_t idx){
                _current_sample_index=idx;
            }
    };
NARUKAMI_END