#include <beet_converter/converter_gltf_parser.h>

#if CHECK_FEATURE(FEATURE_BEET_GLTF_PARSER)
#include <beet_shared/assert.h>
#include <beet_shared/base_64.h>
#include <beet_shared/c_string.h>
#include <beet_shared/filesystem.h>

#include <cstdint>
#include <cstring>
#include <vector>

#include <rapidjson/document.h>

//NOTE:
//      This GLTF parser covers static geo data needed for standard PBR material scenes,
//      it lacks support for PBR extensions & animations, this parser was written with the intent to get a better grips with the GLTF standard
//      I'm also not sure if I would use rapidJSON in the future, while it is very fast, I would like to provide C bindings
//
//      If I write another GLTF parser in te future I would use the GLTF schemas to generate the json parser & the in memory representation (structs).
//      I think the amount of effort that would be spent doing that would be well worth it, as adding extension support would be as simple as defining a spec

//===API================================================================================================================
size_t gltf_component_type_size_lookup(GltfComponentTypesEnum type) {
    switch (type) {
        case GLTF_COMPONENT_UNDEFINED:
            return 0;
        case GLTF_INT_8:
            return sizeof(int8_t);
        case GLTF_UINT_8:
            return sizeof(uint8_t);
        case GLTF_INT_16:
            return sizeof(int16_t);
        case GLTF_UINT_16:
            return sizeof(uint16_t);
        case GLTF_COMPONENT_UNUSED:
            return 0; // likely should be sizeof(int32_t) but int32 is not defined in the spec
        case GLTF_UINT_32:
            return sizeof(uint32_t);
        case GLTF_FLOAT_32:
            return sizeof(float);
    }
    SANITY_CHECK()
    return 0;
}

const char *gltf_component_type_lookup(GltfComponentTypesEnum type) {
    switch (type) {
        case GLTF_COMPONENT_UNDEFINED:
            return "GLTF_COMPONENT_UNDEFINED";
        case GLTF_INT_8:
            return "GLTF_INT_8";
        case GLTF_UINT_8:
            return "GLTF_UINT_8";
        case GLTF_INT_16:
            return "GLTF_INT_16";
        case GLTF_UINT_16:
            return "GLTF_UINT_16";
        case GLTF_COMPONENT_UNUSED:
            return "GLTF_COMPONENT_UNUSED";
        case GLTF_UINT_32:
            return "GLTF_UINT_32";
        case GLTF_FLOAT_32:
            return "GLTF_FLOAT_32";
    }
    SANITY_CHECK()
    return "";
}

const char *gltf_accessor_type_lookup(GltfAccessorType type) {
    switch (type) {
        case GLTF_ACCESSOR_UNDEFINED:
            return "GLTF_ACCESSOR_UNDEFINED";
        case GLTF_SCALAR:
            return "GLTF_SCALAR";
        case GLTF_VEC2:
            return "GLTF_VEC2";
        case GLTF_VEC3:
            return "GLTF_VEC3";
        case GLTF_VEC4:
            return "GLTF_VEC4";
        case GLTF_MAT2:
            return "GLTF_MAT2";
        case GLTF_MAT3:
            return "GLTF_MAT3";
        case GLTF_MAT4:
            return "GLTF_MAT4";
    }
    SANITY_CHECK()
    return "";
}

uint32_t gltf_accessor_type_size_lookup(GltfAccessorType type) {
    switch (type) {
        case GLTF_ACCESSOR_UNDEFINED:
            return 0;
        case GLTF_SCALAR:
            return 1;
        case GLTF_VEC2:
            return 2;
        case GLTF_VEC3:
            return 3;
        case GLTF_VEC4:
        case GLTF_MAT2:
            return 4;
        case GLTF_MAT3:
            return 9;
        case GLTF_MAT4:
            return 16;
    }
    SANITY_CHECK()
    return 0;
}

GltfAccessorType gltf_accessor_type_lookup(const char *str) {
    if (c_str_equal(str, "SCALAR")) {
        return GLTF_SCALAR;
    }
    if (c_str_equal(str, "VEC2")) {
        return GLTF_VEC2;
    }
    if (c_str_equal(str, "VEC3")) {
        return GLTF_VEC3;
    }
    if (c_str_equal(str, "VEC4")) {
        return GLTF_VEC4;
    }
    if (c_str_equal(str, "MAT2")) {
        return GLTF_MAT2;
    }
    if (c_str_equal(str, "MAT3")) {
        return GLTF_MAT3;
    }
    if (c_str_equal(str, "MAT4")) {
        return GLTF_MAT4;
    }
    SANITY_CHECK()
    return GLTF_ACCESSOR_UNDEFINED;
}

GltfAlphaMode gltf_alpha_mode_type_lookup(const char *alphaModeStr) {
    if (c_str_equal(alphaModeStr, "OPAQUE")) {
        return GLTF_ALPHA_MODE_OPAQUE;
    } else if (c_str_equal(alphaModeStr, "MASK")) {
        return GLTF_ALPHA_MODE_MASK;
    } else if (c_str_equal(alphaModeStr, "BLEND")) {
        return GLTF_ALPHA_MODE_BLEND;
    }
    SANITY_CHECK();
    return GLTF_ALPHA_MODE_OPAQUE;
}

GltfIntermediate g_gltfData;

bool parse_gltf_get_binary_data(std::vector<char> &outData, const char *uri, const char *gltfPath) {
    char buildPath[256] = {};
    sprintf(buildPath, "%s", gltfPath);
    const bool result = c_str_replace_after_delim_reverse(buildPath, uri, "/");
    ASSERT(result)

    ASSERT(outData.empty())
    if (fs_file_exists(buildPath)) {
        const size_t fileSize = fs_file_size(buildPath);
        outData.resize(fileSize);
        FILE *fp = nullptr;
        fp = fopen(buildPath, "rb");
        ASSERT(fp != nullptr);
        if (fp) {
            fread(outData.data(), fileSize, 1, fp);
            fclose(fp);
            fp = nullptr;
            return true;
        }
    }
    return false;
}

void parse_gltf_buffers(std::vector<GltfBuffer> &outBuffer, const rapidjson::Value &bufferValue) {
    ASSERT(bufferValue.IsArray())
    outBuffer.reserve(bufferValue.GetArray().Size());
    for (auto const &accessor: bufferValue.GetArray()) {
        GltfBuffer &gltfBuffer = outBuffer.emplace_back();
        for (rapidjson::Value::ConstMemberIterator bufferItr = accessor.MemberBegin(); bufferItr != accessor.MemberEnd(); ++bufferItr) {
            const char *bufferString = bufferItr->name.GetString();
            if (c_str_equal(bufferString, "byteLength")) {
                ASSERT(bufferItr->value.IsUint());
                gltfBuffer.byteLength = bufferItr->value.GetUint();
                continue;
            } else if (c_str_equal(bufferString, "uri")) {
                ASSERT(bufferItr->value.IsString())
                const char *uriWithMetaData = bufferItr->value.GetString();
                const char findTarget[] = "base64,";
                if (const char *foundStr = strstr(uriWithMetaData, findTarget)) {
                    const char *copyStart = foundStr + strlen(findTarget);
                    const size_t copyLen = strlen(copyStart);
                    std::vector<char> base64Data(copyLen + 1, '\0');
                    memcpy(base64Data.data(), copyStart, copyLen);

                    gltfBuffer.binaryData.resize(base64_decode_size(base64Data.data(), base64Data.size() + 1), '\0');
                    base64_decode(base64Data.data(), (uint8_t *) gltfBuffer.binaryData.data());
                } else {
                    bool result = parse_gltf_get_binary_data(gltfBuffer.binaryData, uriWithMetaData, g_gltfData.path);
                    ASSERT(result);
                }
                //TODO: Consider asserting if byteLength and the binaryData size match later on in the code.
                continue;
            }
            NOT_IMPLEMENTED()
        }
    }
}

void parse_gltf_texture(GltfTexture &outTexture, const rapidjson::Value &textureValue) {
    ASSERT(textureValue.IsObject());
    for (rapidjson::Value::ConstMemberIterator baseColorTextureItr = textureValue.MemberBegin();
         baseColorTextureItr != textureValue.MemberEnd(); ++baseColorTextureItr) {
        const char *baseColorTextureString = baseColorTextureItr->name.GetString();
        if (c_str_equal(baseColorTextureString, "index")) {
            ASSERT(baseColorTextureItr->value.IsUint());
            outTexture.index = baseColorTextureItr->value.GetUint();
            continue;
        } else if (c_str_equal(baseColorTextureString, "texCoord")) {
            ASSERT(baseColorTextureItr->value.IsUint());
            outTexture.texCoord = baseColorTextureItr->value.GetUint();
        }
        NOT_IMPLEMENTED();
    }
}

void parse_gltf_materials(std::vector<GltfMaterial> &outMaterials, const rapidjson::Value &materialValue) {
    ASSERT(materialValue.IsArray())
    outMaterials.reserve(materialValue.GetArray().Size());
    for (auto const &accessor: materialValue.GetArray()) {
        GltfMaterial &gltfMaterial = outMaterials.emplace_back();
        for (rapidjson::Value::ConstMemberIterator materialsItr = accessor.MemberBegin(); materialsItr != accessor.MemberEnd(); ++materialsItr) {
            const char *materialsString = materialsItr->name.GetString();
            if (c_str_equal(materialsString, "name")) {
                ASSERT(materialsItr->value.IsString());
                ASSERT(materialsItr->value.GetStringLength() < GLTF_STR_MATERIAL_NAME_SIZE)
                sprintf(gltfMaterial.name, "%s", materialsItr->value.GetString());
                continue;
            } else if (c_str_equal(materialsString, "alphaMode")) {
                ASSERT(materialsItr->value.IsString());
                ASSERT(materialsItr->value.GetStringLength() < GLTF_STR_ALPHA_MODE_NAME_SIZE)
                gltfMaterial.alphaMode = gltf_alpha_mode_type_lookup(materialsItr->value.GetString());
                continue;
            } else if (c_str_equal(materialsString, "alphaCutoff")) {
                ASSERT(materialsItr->value.IsNumber());
                gltfMaterial.alphaCutoff = materialsItr->value.GetFloat();
                continue;
            } else if (c_str_equal(materialsString, "doubleSided")) {
                ASSERT(materialsItr->value.IsBool());
                gltfMaterial.doubleSided = materialsItr->value.GetBool();
                continue;
            } else if (c_str_equal(materialsString, "normalTexture")) {
                ASSERT(materialsItr->value.IsObject());
                parse_gltf_texture(gltfMaterial.normalTexture, materialsItr->value);
                continue;
            } else if (c_str_equal(materialsString, "occlusionTexture")) {
                ASSERT(materialsItr->value.IsObject());
                parse_gltf_texture(gltfMaterial.occlusionTexture, materialsItr->value);
                continue;
            } else if (c_str_equal(materialsString, "emissiveTexture")) {
                ASSERT(materialsItr->value.IsObject());
                parse_gltf_texture(gltfMaterial.emissiveTexture, materialsItr->value);
                continue;
            } else if (c_str_equal(materialsString, "emissiveFactor")) {
                ASSERT(materialsItr->value.IsArray())
                ASSERT(materialsItr->value.GetArray().Size() == 3)
                gltfMaterial.emissiveFactor.r = materialsItr->value[0].GetFloat();
                gltfMaterial.emissiveFactor.g = materialsItr->value[1].GetFloat();
                gltfMaterial.emissiveFactor.b = materialsItr->value[2].GetFloat();
                continue;
            } else if (c_str_equal(materialsString, "pbrMetallicRoughness")) {
                ASSERT(materialsItr->value.IsObject());
                for (rapidjson::Value::ConstMemberIterator pbrMetallicRoughnessItr = materialsItr->value.MemberBegin();
                     pbrMetallicRoughnessItr != materialsItr->value.MemberEnd(); ++pbrMetallicRoughnessItr) {
                    const char *pbrMetallicRoughnessString = pbrMetallicRoughnessItr->name.GetString();
                    if (c_str_equal(pbrMetallicRoughnessString, "metallicFactor")) {
                        ASSERT(pbrMetallicRoughnessItr->value.IsNumber());
                        gltfMaterial.pbrMetallicRoughness.metallicFactor = pbrMetallicRoughnessItr->value.GetFloat();
                        continue;
                    } else if (c_str_equal(pbrMetallicRoughnessString, "roughnessFactor")) {
                        ASSERT(pbrMetallicRoughnessItr->value.IsNumber());
                        gltfMaterial.pbrMetallicRoughness.roughnessFactor = pbrMetallicRoughnessItr->value.GetFloat();
                        continue;
                    } else if (c_str_equal(pbrMetallicRoughnessString, "baseColorTexture")) {
                        ASSERT(pbrMetallicRoughnessItr->value.IsObject());
                        parse_gltf_texture(gltfMaterial.pbrMetallicRoughness.baseColorTexture, pbrMetallicRoughnessItr->value);
                        continue;
                    } else if (c_str_equal(pbrMetallicRoughnessString, "metallicRoughnessTexture")) {
                        ASSERT(pbrMetallicRoughnessItr->value.IsObject());
                        parse_gltf_texture(gltfMaterial.pbrMetallicRoughness.metallicRoughnessTexture, pbrMetallicRoughnessItr->value);
                        continue;
                    } else if (c_str_equal(pbrMetallicRoughnessString, "baseColorFactor")) {
                        ASSERT(pbrMetallicRoughnessItr->value.IsArray())
                        ASSERT(pbrMetallicRoughnessItr->value.GetArray().Size() == 4)
                        gltfMaterial.pbrMetallicRoughness.baseColorFactor.r = pbrMetallicRoughnessItr->value[0].GetFloat();
                        gltfMaterial.pbrMetallicRoughness.baseColorFactor.g = pbrMetallicRoughnessItr->value[1].GetFloat();
                        gltfMaterial.pbrMetallicRoughness.baseColorFactor.b = pbrMetallicRoughnessItr->value[2].GetFloat();
                        gltfMaterial.pbrMetallicRoughness.baseColorFactor.a = pbrMetallicRoughnessItr->value[3].GetFloat();
                        continue;
                    }
                    NOT_IMPLEMENTED();
                }
                continue;
            } else if (c_str_equal(materialsString, "extensions")) {
                ASSERT(materialsItr->value.IsObject());
                for (rapidjson::Value::ConstMemberIterator extItr = materialsItr->value.MemberBegin(); extItr != materialsItr->value.MemberEnd(); ++extItr) {
                    const char *extString = extItr->name.GetString();
                    if (c_str_equal(extString, "KHR_materials_clearcoat")) {

                        NOT_IMPLEMENTED()
                        continue;
                    }
                    NOT_IMPLEMENTED()
                }
                continue;
            }
            NOT_IMPLEMENTED()
        }
    }
}


void parse_gltf_meshes(std::vector<GltfMesh> &outMeshes, const rapidjson::Value &meshValue) {
    ASSERT(meshValue.IsArray())
    outMeshes.reserve(meshValue.GetArray().Size());
    for (auto const &accessor: meshValue.GetArray()) {
        GltfMesh &gltfMesh = outMeshes.emplace_back();

        for (rapidjson::Value::ConstMemberIterator itr = accessor.MemberBegin(); itr != accessor.MemberEnd(); ++itr) {
            const char *jsonString = itr->name.GetString();
            const rapidjson::Value &itrValue = itr->value;

            if (c_str_equal(jsonString, "name")) {
                ASSERT(itr->value.IsString())
                ASSERT(itr->value.GetStringLength() < GLTF_STR_MESH_NAME_SIZE)
                sprintf(gltfMesh.name, "%s", itr->value.GetString());
                continue;
            } else if (c_str_equal(jsonString, "primitives")) {
                ASSERT(itrValue.IsArray())
                for (auto const &primEle: itrValue.GetArray()) {
                    GltfPrimitives &gltfPrimitives = gltfMesh.primitives.emplace_back();
                    for (rapidjson::Value::ConstMemberIterator primItr = primEle.MemberBegin(); primItr != primEle.MemberEnd(); ++primItr) {
                        const rapidjson::Value &primItrValue = primItr->value;
                        const char *primJsonString = primItr->name.GetString();

                        if (c_str_equal(primJsonString, "indices")) {
                            ASSERT(primItrValue.IsUint())
                            gltfPrimitives.indices = primItrValue.GetUint();
                            continue;
                        } else if (c_str_equal(primJsonString, "mode")) {
                            ASSERT(primItrValue.IsInt())
                            gltfPrimitives.mode = GltfTopologyMode(primItrValue.GetInt());
                            continue;
                        } else if (c_str_equal(primJsonString, "material")) {
                            ASSERT(primItrValue.IsUint())
                            gltfPrimitives.material = primItrValue.GetUint();
                            continue;
                        } else if (c_str_equal(primJsonString, "attributes")) {
                            ASSERT(primItrValue.IsObject())
                            for (rapidjson::Value::ConstMemberIterator attribItr = primItrValue.MemberBegin(); attribItr != primItrValue.MemberEnd(); ++attribItr) {

                                const char *attribJsonString = attribItr->name.GetString();
                                const rapidjson::Value &attribItrValue = attribItr->value;

                                if (c_str_equal(attribJsonString, "POSITION")) {
                                    ASSERT(attribItrValue.IsUint())
                                    gltfPrimitives.attrib_position = attribItrValue.GetUint();
                                    continue;
                                } else if (c_str_equal(attribJsonString, "NORMAL")) {
                                    ASSERT(attribItrValue.IsUint())
                                    gltfPrimitives.attrib_normal = attribItrValue.GetUint();
                                    continue;
                                } else if (c_str_equal(attribJsonString, "TANGENT")) {
                                    ASSERT(attribItrValue.IsUint())
                                    gltfPrimitives.attrib_tangent = attribItrValue.GetUint();
                                    continue;
                                } else if (c_str_equal(attribJsonString, "TEXCOORD_0")) {
                                    ASSERT(attribItrValue.IsUint())
                                    gltfPrimitives.attrib_tex_coord_0 = attribItrValue.GetUint();
                                    continue;
                                }
                                NOT_IMPLEMENTED()
                            }
                            continue;
                        }
                        NOT_IMPLEMENTED()
                    }
                }
                continue;
            }
            NOT_IMPLEMENTED()
        }
    }
}

void parse_gltf_nodes(std::vector<GltfNode> &outNodes, const rapidjson::Value &nodeValue) {
    ASSERT(nodeValue.IsArray())
    outNodes.reserve(nodeValue.GetArray().Size());
    for (auto const &accessor: nodeValue.GetArray()) {
        GltfNode &gltfNode = outNodes.emplace_back();

        for (rapidjson::Value::ConstMemberIterator itr = accessor.MemberBegin(); itr != accessor.MemberEnd(); ++itr) {
            const char *jsonString = itr->name.GetString();
            const rapidjson::Value &itrValue = itr->value;

            if (c_str_equal(jsonString, "name")) {
                ASSERT(itr->value.IsString())
                ASSERT(itr->value.GetStringLength() < GLTF_STR_NODE_NAME_SIZE)
                sprintf(gltfNode.name, "%s", itr->value.GetString());
                continue;
            } else if (c_str_equal(jsonString, "mesh")) {
                ASSERT(itrValue.IsUint())
                gltfNode.mesh = itrValue.GetUint();
                continue;
            } else if (c_str_equal(jsonString, "rotation")) {
                ASSERT(itrValue.IsArray())
                ASSERT(itrValue.GetArray().Size() == 4)
                gltfNode.rotation.x = itrValue[0].GetFloat();
                gltfNode.rotation.y = itrValue[1].GetFloat();
                gltfNode.rotation.z = itrValue[2].GetFloat();
                gltfNode.rotation.w = itrValue[3].GetFloat();
                continue;
            } else if (c_str_equal(jsonString, "translation")) {
                ASSERT(itrValue.IsArray())
                ASSERT(itrValue.GetArray().Size() == 3)
                gltfNode.translation.x = itrValue[0].GetFloat();
                gltfNode.translation.y = itrValue[1].GetFloat();
                gltfNode.translation.z = itrValue[2].GetFloat();
                continue;
            } else if (c_str_equal(jsonString, "scale")) {
                ASSERT(itrValue.IsArray())
                ASSERT(itrValue.GetArray().Size() == 3)
                gltfNode.scale.x = itrValue[0].GetFloat();
                gltfNode.scale.y = itrValue[1].GetFloat();
                gltfNode.scale.z = itrValue[2].GetFloat();
                continue;
            } else if (c_str_equal(jsonString, "camera")) {
                ASSERT(itrValue.IsUint())
                gltfNode.camera = itrValue.GetUint();
                continue;
            }
            NOT_IMPLEMENTED()
        }
    }
}

void parse_gltf_scenes(std::vector<GltfScene> &outScenes, const rapidjson::Value &sceneValue) {
    ASSERT(sceneValue.IsArray())
    outScenes.reserve(sceneValue.GetArray().Size());
    for (auto const &accessor: sceneValue.GetArray()) {
        GltfScene &gltfScene = outScenes.emplace_back();

        for (rapidjson::Value::ConstMemberIterator itr = accessor.MemberBegin(); itr != accessor.MemberEnd(); ++itr) {
            const char *jsonString = itr->name.GetString();
            const rapidjson::Value &itrValue = itr->value;

            if (c_str_equal(jsonString, "name")) {
                ASSERT(itr->value.IsString())
                ASSERT(itr->value.GetStringLength() < GLTF_STR_SCENE_NAME_SIZE)
                sprintf(gltfScene.name, "%s", itr->value.GetString());
                continue;
            } else if (c_str_equal(jsonString, "nodes")) {
                ASSERT(itrValue.IsArray())
                for (auto const &nodesEle: itrValue.GetArray()) {
                    gltfScene.nodes.emplace_back(nodesEle.GetUint());
                }
                continue;
            }
            NOT_IMPLEMENTED()
        }
    }
}

void parse_gltf_samplers(std::vector<GltfSamplers> &outSamplers, const rapidjson::Value &samplerValue) {
    ASSERT(samplerValue.IsArray())
    outSamplers.reserve(samplerValue.GetArray().Size());
    for (auto const &samplers: samplerValue.GetArray()) {
        GltfSamplers &gltfSamplers = outSamplers.emplace_back();

        for (rapidjson::Value::ConstMemberIterator itr = samplers.MemberBegin(); itr != samplers.MemberEnd(); ++itr) {
            const char *jsonString = itr->name.GetString();
            const rapidjson::Value &itrValue = itr->value;

            if (c_str_equal(jsonString, "magFilter")) {
                ASSERT(itrValue.IsUint())
                gltfSamplers.magFilter = GltfMagnificationFilter(itrValue.GetUint());
                continue;
            } else if (c_str_equal(jsonString, "minFilter")) {
                ASSERT(itrValue.IsUint())
                gltfSamplers.minFilter = GltfMinificationFilter(itrValue.GetUint());
                continue;
            } else if (c_str_equal(jsonString, "wrapS")) {
                ASSERT(itrValue.IsUint())
                gltfSamplers.wrapS = GltfWrapMode(itrValue.GetUint());
                continue;
            } else if (c_str_equal(jsonString, "wrapT")) {
                ASSERT(itrValue.IsInt())
                gltfSamplers.wrapT = GltfWrapMode(itrValue.GetInt());
                continue;
            }
            NOT_IMPLEMENTED()
        }
    }
}

void parse_gltf_images(std::vector<GltfImages> &outImages, const rapidjson::Value &imageValue) {
    ASSERT(imageValue.IsArray())
    outImages.reserve(imageValue.GetArray().Size());
    for (auto const &texture: imageValue.GetArray()) {
        GltfImages &gltfImages = outImages.emplace_back();

        for (rapidjson::Value::ConstMemberIterator itr = texture.MemberBegin(); itr != texture.MemberEnd(); ++itr) {
            const char *jsonString = itr->name.GetString();
            const rapidjson::Value &itrValue = itr->value;

            if (c_str_equal(jsonString, "bufferView")) {
                ASSERT(itrValue.IsUint())
                gltfImages.bufferView = itrValue.GetUint();
                continue;
            } else if (c_str_equal(jsonString, "mimeType")) {
                ASSERT(itr->value.IsString())
                ASSERT(itr->value.GetStringLength() < GLTF_STR_MEDIA_TYPE_SIZE)
                sprintf(gltfImages.mediaType, "%s", itr->value.GetString());
                continue;
            } else if (c_str_equal(jsonString, "name")) {
                ASSERT(itr->value.IsString())
                ASSERT(itr->value.GetStringLength() < GLTF_STR_MEDIA_NAME_SIZE)
                sprintf(gltfImages.name, "%s", itr->value.GetString());
                continue;
            } else if (c_str_equal(jsonString, "uri")) {
                ASSERT(itr->value.IsString())
                ASSERT(itr->value.GetStringLength() < GLTF_STR_MEDIA_NAME_SIZE)
                sprintf(gltfImages.uri, "%s", itr->value.GetString());
                continue;
            }
            NOT_IMPLEMENTED()
        }
    }
}

void parse_gltf_textures(std::vector<GltfTextures> &outTextures, const rapidjson::Value &textureValue) {
    ASSERT(textureValue.IsArray())
    outTextures.reserve(textureValue.GetArray().Size());
    for (auto const &texture: textureValue.GetArray()) {
        GltfTextures &gltfTextures = outTextures.emplace_back();

        for (rapidjson::Value::ConstMemberIterator itr = texture.MemberBegin(); itr != texture.MemberEnd(); ++itr) {
            const char *jsonString = itr->name.GetString();
            const rapidjson::Value &itrValue = itr->value;

            if (c_str_equal(jsonString, "sampler")) {
                ASSERT(itrValue.IsUint())
                gltfTextures.sampler = itrValue.GetUint();
                continue;
            } else if (c_str_equal(jsonString, "source")) {
                ASSERT(itrValue.IsUint())
                gltfTextures.source = itrValue.GetUint();
                continue;
            }
            NOT_IMPLEMENTED()
        }
    }
}

void parse_gltf_buffer_views(std::vector<GltfBufferViews> &outBufferViews, const rapidjson::Value &bufferViewValue) {
    ASSERT(bufferViewValue.IsArray())
    outBufferViews.reserve(bufferViewValue.GetArray().Size());
    for (auto const &bufferView: bufferViewValue.GetArray()) {
        GltfBufferViews &gltfBufferViews = outBufferViews.emplace_back();

        for (rapidjson::Value::ConstMemberIterator itr = bufferView.MemberBegin(); itr != bufferView.MemberEnd(); ++itr) {
            const char *jsonString = itr->name.GetString();
            const rapidjson::Value &itrValue = itr->value;

            if (c_str_equal(jsonString, "buffer")) {
                ASSERT(itrValue.IsUint())
                gltfBufferViews.bufferIndex = itrValue.GetUint();
                continue;
            } else if (c_str_equal(jsonString, "byteOffset")) {
                ASSERT(itrValue.IsUint())
                gltfBufferViews.byteOffset = itrValue.GetUint();
                continue;
            } else if (c_str_equal(jsonString, "byteLength")) {
                ASSERT(itrValue.IsUint())
                gltfBufferViews.byteLength = itrValue.GetUint();
                continue;
            } else if (c_str_equal(jsonString, "byteStride")) {
                ASSERT(itrValue.IsUint())
                gltfBufferViews.byteStride = itrValue.GetUint();
                continue;
            } else if (c_str_equal(jsonString, "target")) {
                ASSERT(itrValue.IsUint())
                gltfBufferViews.target = itrValue.GetUint();
                continue;
            } else if (c_str_equal(jsonString, "name")) {
                ASSERT(itrValue.IsString())
                ASSERT(itrValue.GetStringLength() < GLTF_STR_BUFFER_VIEWS_NAME_SIZE);
                sprintf(gltfBufferViews.name, "%s", itrValue.GetString());
                continue;
            }
            NOT_IMPLEMENTED()
        }
    }
}

void parse_gltf_cameras(std::vector<GltfCamera> &outCameras, const rapidjson::Value &cameraValue) {
    ASSERT(cameraValue.IsArray())
    outCameras.reserve(cameraValue.GetArray().Size());
    for (auto const &bufferView: cameraValue.GetArray()) {
        GltfCamera &gltfCamera = outCameras.emplace_back();

        for (rapidjson::Value::ConstMemberIterator itr = bufferView.MemberBegin(); itr != bufferView.MemberEnd(); ++itr) {
            const char *jsonString = itr->name.GetString();
            const rapidjson::Value &itrValue = itr->value;

            if (c_str_equal(jsonString, "perspective")) {
                ASSERT(itrValue.IsObject());
                for (rapidjson::Value::ConstMemberIterator perspectiveItr = itrValue.MemberBegin(); perspectiveItr != itrValue.MemberEnd(); ++perspectiveItr) {
                    const rapidjson::Value &perspectiveItrValue = perspectiveItr->value;

                    const char *jsonString = perspectiveItr->name.GetString();
                    if (c_str_equal(jsonString, "yfov")) {
                        ASSERT(perspectiveItrValue.IsNumber());
                        gltfCamera.perspective.yFov = perspectiveItrValue.GetFloat();
                        continue;
                    } else if (c_str_equal(jsonString, "aspectRatio")) {
                        ASSERT(perspectiveItrValue.IsNumber());
                        gltfCamera.perspective.aspectRatio = perspectiveItrValue.GetFloat();
                        continue;
                    } else if (c_str_equal(jsonString, "znear")) {
                        ASSERT(perspectiveItrValue.IsNumber());
                        gltfCamera.zNear = perspectiveItrValue.GetFloat();
                        continue;
                    } else if (c_str_equal(jsonString, "zfar")) {
                        ASSERT(perspectiveItrValue.IsNumber());
                        gltfCamera.zFar = perspectiveItrValue.GetFloat();
                        continue;
                    }
                    NOT_IMPLEMENTED()
                }
                continue;
            } else if (c_str_equal(jsonString, "orthographic")) {
                ASSERT(itrValue.IsNumber());
                gltfCamera.zNear = itrValue.GetFloat();
                continue;
            } else if (c_str_equal(jsonString, "type")) {
                ASSERT(itrValue.IsString());
                if (c_str_equal("perspective", itrValue.GetString())) {
                    gltfCamera.cameraType = GLTF_CAMERA_TYPE_PERSPECTIVE;
                    continue;
                } else if (c_str_equal("orthographic", itrValue.GetString())) {
                    gltfCamera.cameraType = GLTF_CAMERA_TYPE_ORTHOGRAPHIC;
                    continue;
                }
                NOT_IMPLEMENTED();
                continue;
            } else if (c_str_equal(jsonString, "name")) {
                ASSERT(itrValue.IsString());
                ASSERT(itrValue.GetStringLength() < GLTF_STR_CAMERA_NAME_SIZE);
                sprintf(gltfCamera.name, "%s", itrValue.GetString());
                continue;
            }
            NOT_IMPLEMENTED()
        }
    }
}

void parse_gltf_extensions_used(std::vector<GltfUsedExtension> &outUsedExtensions, const rapidjson::Value &usedExtensionsValue) {
    ASSERT(usedExtensionsValue.IsArray())
    outUsedExtensions.reserve(usedExtensionsValue.GetArray().Size());
    for (auto const &usedExtension: usedExtensionsValue.GetArray()) {
        GltfUsedExtension &gltfUsedExtension = outUsedExtensions.emplace_back();
        ASSERT(usedExtension.IsString())
        ASSERT(usedExtension.GetStringLength() < GLTF_STR_EXTENSION_NAME_SIZE);
        sprintf(gltfUsedExtension.itemName, "%s", usedExtension.GetString());
    }
}

void parse_gltf_accessors(std::vector<GltfAccessor> &outAccessors, const rapidjson::Value &accessorsValue) {
    ASSERT(accessorsValue.IsArray())
    outAccessors.reserve(accessorsValue.GetArray().Size());
    for (auto const &accessor: accessorsValue.GetArray()) {
        GltfAccessor &gltfAccessor = outAccessors.emplace_back();

        for (rapidjson::Value::ConstMemberIterator itr = accessor.MemberBegin(); itr != accessor.MemberEnd(); ++itr) {
            const char *jsonString = itr->name.GetString();
            const rapidjson::Value &itrValue = itr->value;

            if (c_str_equal(jsonString, "componentType")) {
                ASSERT(itrValue.IsUint())
                gltfAccessor.componentType = GltfComponentTypesEnum(itrValue.GetUint());
                continue;
            } else if (c_str_equal(jsonString, "type")) {
                ASSERT(itrValue.IsString())
                gltfAccessor.accessorType = gltf_accessor_type_lookup(itrValue.GetString());
                continue;
            } else if (c_str_equal(jsonString, "bufferView")) {
                ASSERT(itrValue.IsUint())
                gltfAccessor.bufferViewIndex = itrValue.GetUint();
                continue;
            } else if (c_str_equal(jsonString, "byteOffset")) {
                ASSERT(itrValue.IsUint())
                gltfAccessor.byteOffset = itrValue.GetUint();
                continue;
            } else if (c_str_equal(jsonString, "count")) {
                ASSERT(itrValue.IsUint())
                gltfAccessor.count = itrValue.GetUint();
                continue;
            } else if (c_str_equal(jsonString, "min") || c_str_equal(jsonString, "max")) {
                ASSERT(itrValue.IsArray())
                for (auto const &minMaxEle: itrValue.GetArray()) {
                    GltfComponentType minMaxVal = {};
                    switch (gltfAccessor.componentType) {
                        case GLTF_COMPONENT_UNDEFINED:
                        case GLTF_COMPONENT_UNUSED: SANITY_CHECK()
                        case GLTF_INT_8:
                            minMaxVal.int8 = int8_t(minMaxEle.GetInt());
                        case GLTF_INT_16:
                            minMaxVal.int16 = int16_t(minMaxEle.GetInt());
                        case GLTF_UINT_8:
                            minMaxVal.uint8 = uint8_t(minMaxEle.GetUint());
                        case GLTF_UINT_16:
                            minMaxVal.uint16 = uint16_t(minMaxEle.GetUint());
                        case GLTF_UINT_32:
                            minMaxVal.uint32 = minMaxEle.GetUint();
                        case GLTF_FLOAT_32:
                            minMaxVal.float32 = minMaxEle.GetFloat();
                    }

                    if (c_str_equal(jsonString, "min")) {
                        gltfAccessor.min.emplace_back(minMaxVal);
                    }
                    if (c_str_equal(jsonString, "max")) {
                        gltfAccessor.max.emplace_back(minMaxVal);
                    }
                }
                continue;
            } else if (c_str_equal(jsonString, "name")) {
                ASSERT(itrValue.IsString())
                ASSERT(itrValue.GetStringLength() < GLTF_STR_ACCESSOR_NAME_SIZE);
                sprintf(gltfAccessor.name, "%s", itrValue.GetString());
                continue;
            }
            NOT_IMPLEMENTED()
        }
    }
}

void parse_gltf_asset(GltfAsset &outAsset, const rapidjson::Value &jsonValue) {
    ASSERT(jsonValue.IsObject())
    for (rapidjson::Value::ConstMemberIterator itr = jsonValue.MemberBegin(); itr != jsonValue.MemberEnd(); ++itr) {
        const char *jsonString = itr->name.GetString();
        log_verbose(MSG_CONVERTER, "\t%s\n", jsonString)

        if (c_str_equal(jsonString, "generator")) {
            ASSERT(itr->value.IsString())
            ASSERT(itr->value.GetStringLength() < GLTF_STR_GENERATOR_SIZE)
            sprintf(outAsset.generator, "%s", itr->value.GetString());
            continue;
        } else if (c_str_equal(jsonString, "version")) {
            ASSERT(itr->value.IsString())
            ASSERT(itr->value.GetStringLength() < GLTF_STR_VERSION_SIZE)
            sprintf(outAsset.version, "%s", itr->value.GetString());
            continue;
        } else if (c_str_equal(jsonString, "copyright")) {
            ASSERT(itr->value.IsString())
            ASSERT(itr->value.GetStringLength() < GLTF_STR_COPYRIGHT_SIZE)
            sprintf(outAsset.copyright, "%s", itr->value.GetString());
            continue;
        }
        NOT_IMPLEMENTED()
    }
}


void gltf_dump_intermediate(const GltfIntermediate &gltfData) {
    printf("start: dumping gltf intermediate:\n\n");

    printf("Asset:\n");
    printf(" Generator: %s\n", gltfData.asset.generator);
    printf(" Version: %s\n", gltfData.asset.version);
    printf("\n");

    printf("Accessor Array: [%zu]\n", gltfData.accessors.size());
    for (const GltfAccessor &accessor: gltfData.accessors) {
        printf(" componentType: %u - %s\n", accessor.componentType, gltf_component_type_lookup(accessor.componentType));
        printf(" accessorType: %u - %s - [ %u ]\n", accessor.accessorType, gltf_accessor_type_lookup(accessor.accessorType), gltf_accessor_type_size_lookup(accessor.accessorType));
        printf(" bufferViewIndex: %u \n", accessor.bufferViewIndex);
        printf(" byteOffset: %u \n", accessor.byteOffset);
        printf(" count: %u\n", accessor.count);

        const auto PrintAccessor = [](const std::vector<GltfComponentType> &data, const GltfComponentTypesEnum componentType) -> void {
            for (size_t i = 0; i < data.size(); ++i) {
                switch (componentType) {
                    case GLTF_INT_8:
                        printf("%" PRIi8 " ", data[i].int8);
                        break;
                    case GLTF_INT_16:
                        printf("%" PRIi16 " ", data[i].int16);
                        break;
                    case GLTF_UINT_8:
                        printf("%" PRIu8 " ", data[i].uint8);
                        break;
                    case GLTF_UINT_16:
                        printf("%" PRIu16 " ", data[i].uint16);
                        break;
                    case GLTF_UINT_32:
                        printf("%" PRIu32 " ", data[i].uint32);
                        break;
                    case GLTF_FLOAT_32:
                        printf("%f ", data[i].float32);
                        break;
                    case GLTF_COMPONENT_UNDEFINED:
                    case GLTF_COMPONENT_UNUSED:
                    default: SANITY_CHECK()
                }
            }
        };

        printf(" min: ");
        PrintAccessor(accessor.min, accessor.componentType);
        printf("\n");

        printf(" max: ");
        PrintAccessor(accessor.max, accessor.componentType);
        printf("\n");
        printf("\n");
    }

    printf("bufferViews Array: [%zu]\n", gltfData.bufferViews.size());
    for (const GltfBufferViews &bufferViews: gltfData.bufferViews) {
        printf(" bufferIndex: %u\n", bufferViews.bufferIndex);
        printf(" byteOffset: %u\n", bufferViews.byteOffset);
        printf(" byteLength: %u \n", bufferViews.byteLength);
        printf(" target: %u \n", bufferViews.target);
        printf("\n");
    }

    printf("bufferViews Array: [%zu]\n", gltfData.textures.size());
    for (const GltfTextures &texture: gltfData.textures) {
        printf(" buffer: %u\n", texture.sampler);
        printf(" byteOffset: %u\n", texture.source);
        printf("\n");
    }

    printf("samplers Array: [%zu]\n", gltfData.samplers.size());
    for (const GltfSamplers &samplers: gltfData.samplers) {
        printf(" magFilter: %u\n", samplers.magFilter);
        printf(" minFilter: %u\n", samplers.minFilter);
        printf("\n");
    }

    printf("Images Array: [%zu]\n", gltfData.images.size());
    for (const GltfImages &images: gltfData.images) {
        printf(" bufferView: %u\n", images.bufferView);
        printf(" name: %s\n", images.name);
        printf(" mediaType: %s\n", images.mediaType);
        printf(" uri: %s\n", images.uri);
        printf("\n");
    }

    printf("Scene Count: %u\n", gltfData.sceneCount);
    printf("\n");

    printf("Scenes Array: [%zu]\n", gltfData.scenes.size());
    for (const GltfScene &scene: gltfData.scenes) {
        printf(" name: '%s'\n", scene.name);
        printf(" Nodes Array: [%zu]\n", scene.nodes.size());
        for (const uint32_t &node: scene.nodes) {
            printf("  %u\n", node);
        }
        printf("\n");
    }

    printf("Nodes Array: [%zu]\n", gltfData.nodes.size());
    for (const GltfNode &node: gltfData.nodes) {
        printf(" name: '%s'\n", node.name);
        printf(" meshIndex: %u\n", node.mesh);
        printf(" translation : [ %f , %f , %f ]\n", node.translation.x, node.translation.y, node.translation.z);
        printf(" rotation : [ %f , %f , %f , %f ]\n", node.rotation.x, node.rotation.y, node.rotation.z, node.rotation.w);
        printf(" scale : [ %f , %f , %f ]\n", node.scale.x, node.scale.y, node.scale.z);
        printf("\n");
    }

    printf("Meshes Array: [%zu]\n", gltfData.meshes.size());
    for (const GltfMesh &mesh: gltfData.meshes) {
        printf(" name: '%s'\n", mesh.name);
        printf(" Primitives Array: [%zu]\n", mesh.primitives.size());
        for (const GltfPrimitives &primitive: mesh.primitives) {
            printf("  indices %u\n", primitive.indices);
            printf("  material %u\n", primitive.material);
            printf("  attribute: position %u\n", primitive.attrib_position);
            printf("  attribute: normal %u\n", primitive.attrib_normal);
            printf("  attribute: tangent %u\n", primitive.attrib_tangent);
            printf("  attribute: tex coord %u\n", primitive.attrib_tex_coord_0);
        }
        printf("\n");
    }

    printf("Material Array: [%zu]\n", gltfData.materials.size());
    for (const GltfMaterial &material: gltfData.materials) {
        printf(" name: '%s'\n", material.name);
        printf(" doubleSided: %u\n", material.doubleSided);
        printf(" pbrMetallicRoughness:\n");
        printf("  baseColorTexture index: %u\n", material.pbrMetallicRoughness.baseColorTexture.index);
        printf("  baseColorTexture tex coord: %u\n", material.pbrMetallicRoughness.baseColorTexture.texCoord);
        printf("  metallicFactor: %f\n", material.pbrMetallicRoughness.metallicFactor);
        printf("  roughnessFactor: %f\n", material.pbrMetallicRoughness.roughnessFactor);

        printf("\n");
    }

    printf("Buffers Array: [%zu]\n", gltfData.buffers.size());
    for (const GltfBuffer &buffer: gltfData.buffers) {
        printf(" byteLength: %zu\n", buffer.byteLength);
        printf(" uriData:( binary Data size) %zu\n", buffer.binaryData.size());
        for (const char c: buffer.binaryData) {
            printf("%c", c);
        }
        printf("\n");
        printf("\n");
    }

    printf("end: dumping gltf intermediate:\n");
    printf("\n");
}

void *gltf_pull_out_binary_data_alloc(const uint32_t inAccessorIndex, size_t &outCount, GltfAccessorType &outAccessor, GltfComponentTypesEnum &outType) {
    if (inAccessorIndex != GLTF_INDEX_NOT_SET) {
        const GltfAccessor &accessor = g_gltfData.accessors[inAccessorIndex];
        const GltfBufferViews &bufferView = g_gltfData.bufferViews[accessor.bufferViewIndex];
        const GltfBuffer &buffer = g_gltfData.buffers[bufferView.bufferIndex];
        outAccessor = accessor.accessorType;
        outType = accessor.componentType;
        outCount = accessor.count * gltf_accessor_type_size_lookup(accessor.accessorType);
        const size_t allocSize = outCount * gltf_component_type_size_lookup(accessor.componentType);
        ASSERT(allocSize <= bufferView.byteLength); // sometimes we subview into a buffer.

        void *outData = mem_zalloc(allocSize);
        ASSERT(outData != nullptr)
        if (bufferView.byteStride == 0) {
            memcpy(outData, &buffer.binaryData[0] + bufferView.byteOffset + accessor.byteOffset, allocSize);
        } else {
            for (int i = 0; i < accessor.count; ++i) {
                size_t cpySize = gltf_accessor_type_size_lookup(accessor.accessorType) * gltf_component_type_size_lookup(accessor.componentType);
                ASSERT(bufferView.byteStride == cpySize)
                const char *cpyStart = (&buffer.binaryData[0] + bufferView.byteOffset + accessor.byteOffset) + (i * cpySize);
                const char *destStart = ((char *) outData + (i * cpySize));
                size_t strideOffset = (i * (accessor.byteOffset));
                memcpy((void *) destStart, cpyStart + strideOffset, cpySize);
            }
        }
        return outData;
    }
    return nullptr;
}

void gltf_parse_json(const char *gltfPath, const char *targetWriteDir, AssetPackage &outPackage) {
    sprintf(g_gltfData.path, "%s", gltfPath);
    rapidjson::Document document;  // Default template parameter uses UTF8 and MemoryPoolAllocator.
    FILE *fp = nullptr;
    fp = fopen(g_gltfData.path, "rb");
    std::vector<char> jsonData;
    if (fp) {
        size_t fileSize = fs_file_size(g_gltfData.path);
        jsonData.resize(fileSize + 1, '\0');
        fread(jsonData.data(), fileSize, 1, fp);
        fclose(fp);
    }

    if (document.Parse(jsonData.data()).HasParseError()) {
        SANITY_CHECK()
    }

    ASSERT(document.IsObject())
    for (rapidjson::Value::ConstMemberIterator itr = document.MemberBegin(); itr != document.MemberEnd(); ++itr) {
        const char *jsonString = itr->name.GetString();
        if (c_str_equal(jsonString, "scene")) {
            ASSERT(itr->value.IsUint())
            g_gltfData.sceneCount = itr->value.GetUint();
            continue;
        } else if (c_str_equal(jsonString, "scenes")) {
            parse_gltf_scenes(g_gltfData.scenes, itr->value);
            continue;
        } else if (c_str_equal(jsonString, "nodes")) {
            parse_gltf_nodes(g_gltfData.nodes, itr->value);
            continue;
        } else if (c_str_equal(jsonString, "meshes")) {
            parse_gltf_meshes(g_gltfData.meshes, itr->value);
            continue;
        } else if (c_str_equal(jsonString, "materials")) {
            parse_gltf_materials(g_gltfData.materials, itr->value);
            continue;
        } else if (c_str_equal(jsonString, "asset")) {
            parse_gltf_asset(g_gltfData.asset, itr->value);
            continue;
        } else if (c_str_equal(jsonString, "accessors")) {
            parse_gltf_accessors(g_gltfData.accessors, itr->value);
            continue;
        } else if (c_str_equal(jsonString, "bufferViews")) {
            parse_gltf_buffer_views(g_gltfData.bufferViews, itr->value);
            continue;
        } else if (c_str_equal(jsonString, "textures")) {
            parse_gltf_textures(g_gltfData.textures, itr->value);
            continue;
        } else if (c_str_equal(jsonString, "samplers")) {
            parse_gltf_samplers(g_gltfData.samplers, itr->value);
            continue;
        } else if (c_str_equal(jsonString, "images")) {
            parse_gltf_images(g_gltfData.images, itr->value);
            continue;
        } else if (c_str_equal(jsonString, "buffers")) {
            parse_gltf_buffers(g_gltfData.buffers, itr->value);
            continue;
        } else if (c_str_equal(jsonString, "cameras")) {
            parse_gltf_cameras(g_gltfData.cameras, itr->value);
            continue;
        } else if (c_str_equal(jsonString, "extensionsUsed")) {
            parse_gltf_extensions_used(g_gltfData.usedExtensions, itr->value);
            continue;
        }

        NOT_IMPLEMENTED(); // Parser is still IN_DEV, will implement missing features as and when I run into issues.
    }

//    gltf_dump_intermediate(g_gltfData);

    //TODO: Do this for each scene/node/mesh.

//    for (size_t nodeIndex = 0; nodeIndex < g_gltfData.nodes.size(); ++nodeIndex) {
//        GltfNode &node = g_gltfData.nodes[nodeIndex];
//    }

    for (size_t primIndex = 0; primIndex < g_gltfData.meshes[0].primitives.size(); ++primIndex) {
        GltfPrimitives &currentPrimitive = g_gltfData.meshes[0].primitives[primIndex];

        size_t indicesCount = 0;
        GltfAccessorType indicesAccessorType = {};
        GltfComponentTypesEnum indicesComponentType = {};
        void *indices = gltf_pull_out_binary_data_alloc(currentPrimitive.indices, indicesCount, indicesAccessorType, indicesComponentType);

        size_t tangentCount = 0;
        GltfAccessorType tangentAccessorType;
        GltfComponentTypesEnum tangentComponentType;
        void *tangents = gltf_pull_out_binary_data_alloc(currentPrimitive.attrib_tangent, tangentCount, tangentAccessorType, tangentComponentType);

        size_t normalCount = 0;
        GltfAccessorType normalAccessorType;
        GltfComponentTypesEnum normalComponentType;
        void *normals = gltf_pull_out_binary_data_alloc(currentPrimitive.attrib_normal, normalCount, normalAccessorType, normalComponentType);

        size_t uvCount = 0;
        GltfAccessorType uvAccessorType;
        GltfComponentTypesEnum uvComponentType;
        void *uvs = gltf_pull_out_binary_data_alloc(currentPrimitive.attrib_tex_coord_0, uvCount, uvAccessorType, uvComponentType);

        size_t positionCount = 0;
        GltfAccessorType positionAccessorType;
        GltfComponentTypesEnum positionComponentType;
        void *positions = gltf_pull_out_binary_data_alloc(currentPrimitive.attrib_position, positionCount, positionAccessorType, positionComponentType);

        std::vector<GfxVertex> raw_verts(positionCount / gltf_accessor_type_size_lookup(positionAccessorType));
        std::vector<uint32_t> raw_indices;

        for (size_t i = 0; i < raw_verts.size(); i++) {
            GfxVertex &vertRef = raw_verts[i];

            if (positionCount > 0) {
                const size_t elementCount = gltf_accessor_type_size_lookup(positionAccessorType);
                ASSERT(elementCount == 3)
                ASSERT(positionComponentType == GLTF_FLOAT_32);
                vertRef.pos.x = ((float *) positions)[(i * elementCount) + 0];
                vertRef.pos.y = ((float *) positions)[(i * elementCount) + 1];
                vertRef.pos.z = ((float *) positions)[(i * elementCount) + 2];
            }
            if (normalCount > 0) {
                const size_t elementCount = gltf_accessor_type_size_lookup(normalAccessorType);
                ASSERT(elementCount == 3)
                ASSERT(positionComponentType == GLTF_FLOAT_32);
                vertRef.normal.x = ((float *) normals)[(i * elementCount) + 0];
                vertRef.normal.y = ((float *) normals)[(i * elementCount) + 1];
                vertRef.normal.z = ((float *) normals)[(i * elementCount) + 1];
            }
            if (uvCount > 0) {
                const size_t elementCount = gltf_accessor_type_size_lookup(uvAccessorType);
                ASSERT(elementCount == 2)
                ASSERT(positionComponentType == GLTF_FLOAT_32);
                vertRef.uv.x = ((float *) uvs)[(i * elementCount) + 0];
                vertRef.uv.y = ((float *) uvs)[(i * elementCount) + 1];
            }
            if (tangentCount > 0) {
                const size_t elementCount = gltf_accessor_type_size_lookup(tangentAccessorType);
                ASSERT(elementCount == 4)
                ASSERT(positionComponentType == GLTF_FLOAT_32);
                //TODO: GfxVertex does not taken in tangents (currently)
            }
            {
                //TODO: COLOUR
            }
        }

        for (size_t i = 0; i < indicesCount; i += 1) {
            const size_t elementCount = gltf_accessor_type_size_lookup(indicesAccessorType);
            ASSERT(elementCount == 1);
            uint32_t &idx = raw_indices.emplace_back();
            switch (indicesComponentType) {
                case GLTF_UINT_8 :
                    idx = ((uint8_t *) indices)[i];
                    break;
                case GLTF_UINT_16 :
                    idx = ((uint16_t *) indices)[i];
                    break;
                case GLTF_UINT_32 :
                    idx = ((uint32_t *) indices)[i];
                    break;
                case GLTF_COMPONENT_UNDEFINED:
                case GLTF_COMPONENT_UNUSED: // likely int32_t
                case GLTF_INT_8:
                case GLTF_INT_16:
                case GLTF_FLOAT_32: NOT_IMPLEMENTED() // I don't expect we to need to support these.
                    break;
            }
        }

        const RawMesh rawMesh = {
                raw_verts.data(),
                raw_indices.data(),
                static_cast<uint32_t>(raw_verts.size()),
                static_cast<uint32_t>(raw_indices.size()),
        };
        RawMaterial &material = outPackage.materials.emplace_back();
        strcpy(material.albedoPath, targetWriteDir);
        char *lastDir = c_str_search_reverse(material.albedoPath, "/") + 1;

        char *albedoPath = g_gltfData.images[g_gltfData.textures[g_gltfData.materials[currentPrimitive.material].pbrMetallicRoughness.baseColorTexture.index].source].uri;
        strcpy(lastDir, albedoPath);
        c_string_replace_extension(material.albedoPath, ".dds");
        GfxMesh &curr = outPackage.meshes.emplace_back(); // maybe this should be a RawMesh instead. then we cal call create immediate from entity_builder.
        PackageEntry &currentEntry = outPackage.packageTable.emplace_back();
        currentEntry.meshIndex = outPackage.packageTable.size() - 1;
        currentEntry.materialIndex = outPackage.packageTable.size() - 1;
        gfx_mesh_create_immediate(rawMesh, curr);

        if (positionCount > 0) {
            mem_free(positions);
        }
        if (uvCount > 0) {
            mem_free(uvs);
        }
        if (normalCount > 0) {
            mem_free(normals);
        }
        if (tangentCount > 0) {
            mem_free(tangents);
        }
        if (indicesCount > 0) {
            mem_free(indices);
        }
    }
}
//======================================================================================================================

#endif //CHECK_FEATURE(FEATURE_BEET_GLTF_PARSER)
