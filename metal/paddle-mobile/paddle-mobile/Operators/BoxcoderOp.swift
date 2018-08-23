///* Copyright (c) 2018 PaddlePaddle Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License. */

import Foundation

class BoxcoderParam<P: PrecisionType>: OpParam {
    typealias ParamPrecisionType = P
    required init(opDesc: OpDesc, inScope: Scope) throws {
        do {
            priorBox = try BoxcoderParam.getFirstTensor(key: "PriorBox", map: opDesc.inputs, from: inScope)
            priorBoxVar = try BoxcoderParam.getFirstTensor(key: "PriorBoxVar", map: opDesc.inputs, from: inScope)
            targetBox = try BoxcoderParam.getFirstTensor(key: "TargetBox", map: opDesc.inputs, from: inScope)
            output = try BoxcoderParam.getFirstTensor(key: "OutputBox", map: opDesc.outputs, from: inScope)
            codeType = try BoxcoderParam.getAttr(key: "code_type", attrs: opDesc.attrs)
            boxNormalized = try BoxcoderParam.getAttr(key: "box_normalized", attrs: opDesc.attrs)
        } catch let error {
            throw error
        }
        assert(priorBox.transpose == [0, 1, 2, 3])
        assert(priorBoxVar.transpose == [0, 1, 2, 3])
        assert(targetBox.transpose == [0, 1, 2, 3])
        assert(codeType == "decode_center_size") // encode_center_size is not implemented
        assert((targetBox.tensorDim.cout() == 3) && (targetBox.tensorDim[0] == 1)) // N must be 1 (only handle batch size = 1)
    }
    let priorBox: Texture<P>
    let priorBoxVar: Texture<P>
    let targetBox: Texture<P>
    var output: Texture<P>
    let codeType: String
    let boxNormalized: Bool
}

class BoxcoderOp<P: PrecisionType>: Operator<BoxcoderKernel<P>, BoxcoderParam<P>>, Runable, Creator, InferShaperable{
    
    func inferShape() {
//        para.output.dim = para.input.dim
    }
    
    typealias OpType = BoxcoderOp<P>
    func runImpl(device: MTLDevice, buffer: MTLCommandBuffer) throws {
        do {
            try kernel.compute(commandBuffer: buffer, param: para)
        } catch let error {
            throw error
        }
    }
}



