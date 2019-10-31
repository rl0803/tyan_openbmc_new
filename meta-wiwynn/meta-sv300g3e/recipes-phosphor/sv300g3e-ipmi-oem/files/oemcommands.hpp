/*
// Copyright (c) 2019 Wiwynn Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
*/

#pragma once

static constexpr ipmi_netfn_t netFnSv300g3eOEM1 = 0x2E;
static constexpr ipmi_netfn_t netFnSv300g3eOEM2 = 0x30;
static constexpr ipmi_netfn_t netFnSv300g3eOEM3 = 0x3E;

static const size_t maxPatternLen = 256;   

enum ipmi_sv300g3e_oem3_cmds : uint8_t
{
    CMD_SET_SOL_PATTERN = 0xB2,
    CMD_GET_SOL_PATTERN = 0xB3,

};

typedef struct
{
    uint8_t patternIdx;
    char data[maxPatternLen];
}__attribute__((packed)) SetSolPatternCmdReq;

typedef struct
{
    uint8_t patternIdx;
}__attribute__((packed)) GetSolPatternCmdReq;

typedef struct
{
    char data[maxPatternLen];
}__attribute__((packed)) GetSolPatternCmdRes;

