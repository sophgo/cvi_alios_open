/*
 * Copyright (c) 2022, sakumisu
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef USB_MTP_H
#define USB_MTP_H

#define USB_MTP_CLASS 0x06

#define USB_MTP_SUB_CLASS 0x01U
#define USB_MTP_PROTOCOL  0x01U

#define USB_MTP_REQUEST_CANCEL             0x64U
#define USB_MTP_REQUEST_GET_EXT_EVENT_DATA 0x65U
#define USB_MTP_REQUEST_RESET              0x66U
#define USB_MTP_REQUEST_GET_DEVICE_STATUS  0x67U

/*
 * MTP Class specification Revision 1.1
 * Appendix B. Object Properties
 */
/* MTP OBJECT PROPERTIES supported*/
#define USB_MTP_OB_PROP_STORAGE_ID                          0xDC01U
#define USB_MTP_OB_PROP_OBJECT_FORMAT                       0xDC02U
#define USB_MTP_OB_PROP_PROTECTION_STATUS                   0xDC03U
#define USB_MTP_OB_PROP_OBJECT_SIZE                         0xDC04U
#define USB_MTP_OB_PROP_ASSOC_TYPE                          0xDC05U
#define USB_MTP_OB_PROP_ASSOC_DESC                          0xDC06U
#define USB_MTP_OB_PROP_OBJ_FILE_NAME                       0xDC07U
#define USB_MTP_OB_PROP_DATE_CREATED                        0xDC08U
#define USB_MTP_OB_PROP_DATE_MODIFIED                       0xDC09U
#define USB_MTP_OB_PROP_KEYWORDS                            0xDC0AU
#define USB_MTP_OB_PROP_PARENT_OBJECT                       0xDC0BU
#define USB_MTP_OB_PROP_ALLOWED_FOLD_CONTENTS               0xDC0CU
#define USB_MTP_OB_PROP_HIDDEN                              0xDC0DU
#define USB_MTP_OB_PROP_SYSTEM_OBJECT                       0xDC0EU
#define USB_MTP_OB_PROP_PERS_UNIQ_OBJ_IDEN                  0xDC41U
#define USB_MTP_OB_PROP_SYNCID                              0xDC42U
#define USB_MTP_OB_PROP_PROPERTY_BAG                        0xDC43U
#define USB_MTP_OB_PROP_NAME                                0xDC44U
#define USB_MTP_OB_PROP_CREATED_BY                          0xDC45U
#define USB_MTP_OB_PROP_ARTIST                              0xDC46U
#define USB_MTP_OB_PROP_DATE_AUTHORED                       0xDC47U
#define USB_MTP_OB_PROP_DESCRIPTION                         0xDC48U
#define USB_MTP_OB_PROP_URL_REFERENCE                       0xDC49U
#define USB_MTP_OB_PROP_LANGUAGELOCALE                      0xDC4AU
#define USB_MTP_OB_PROP_COPYRIGHT_INFORMATION               0xDC4BU
#define USB_MTP_OB_PROP_SOURCE                              0xDC4CU
#define USB_MTP_OB_PROP_ORIGIN_LOCATION                     0xDC4DU
#define USB_MTP_OB_PROP_DATE_ADDED                          0xDC4EU
#define USB_MTP_OB_PROP_NON_CONSUMABLE                      0xDC4FU
#define USB_MTP_OB_PROP_CORRUPTUNPLAYABLE                   0xDC50U
#define USB_MTP_OB_PROP_PRODUCERSERIALNUMBER                0xDC51U
#define USB_MTP_OB_PROP_REPRESENTATIVE_SAMPLE_FORMAT        0xDC81U
#define USB_MTP_OB_PROP_REPRESENTATIVE_SAMPLE_SIZE          0xDC82U
#define USB_MTP_OB_PROP_REPRESENTATIVE_SAMPLE_HEIGHT        0xDC83U
#define USB_MTP_OB_PROP_REPRESENTATIVE_SAMPLE_WIDTH         0xDC84U
#define USB_MTP_OB_PROP_REPRESENTATIVE_SAMPLE_DURATION      0xDC85U
#define USB_MTP_OB_PROP_REPRESENTATIVE_SAMPLE_DATA          0xDC86U
#define USB_MTP_OB_PROP_WIDTH                               0xDC87U
#define USB_MTP_OB_PROP_HEIGHT                              0xDC88U
#define USB_MTP_OB_PROP_DURATION                            0xDC89U
#define USB_MTP_OB_PROP_RATING                              0xDC8AU
#define USB_MTP_OB_PROP_TRACK                               0xDC8BU
#define USB_MTP_OB_PROP_GENRE                               0xDC8CU
#define USB_MTP_OB_PROP_CREDITS                             0xDC8DU
#define USB_MTP_OB_PROP_LYRICS                              0xDC8EU
#define USB_MTP_OB_PROP_SUBSCRIPTION_CONTENT_ID             0xDC8FU
#define USB_MTP_OB_PROP_PRODUCED_BY                         0xDC90U
#define USB_MTP_OB_PROP_USE_COUNT                           0xDC91U
#define USB_MTP_OB_PROP_SKIP_COUNT                          0xDC92U
#define USB_MTP_OB_PROP_LAST_ACCESSED                       0xDC93U
#define USB_MTP_OB_PROP_PARENTAL_RATING                     0xDC94U
#define USB_MTP_OB_PROP_META_GENRE                          0xDC95U
#define USB_MTP_OB_PROP_COMPOSER                            0xDC96U
#define USB_MTP_OB_PROP_EFFECTIVE_RATING                    0xDC97U
#define USB_MTP_OB_PROP_SUBTITLE                            0xDC98U
#define USB_MTP_OB_PROP_ORIGINAL_RELEASE_DATE               0xDC99U
#define USB_MTP_OB_PROP_ALBUM_NAME                          0xDC9AU
#define USB_MTP_OB_PROP_ALBUM_ARTIST                        0xDC9BU
#define USB_MTP_OB_PROP_MOOD                                0xDC9CU
#define USB_MTP_OB_PROP_DRM_STATUS                          0xDC9DU
#define USB_MTP_OB_PROP_SUB_DESCRIPTION                     0xDC9EU
#define USB_MTP_OB_PROP_IS_CROPPED                          0xDCD1U
#define USB_MTP_OB_PROP_IS_COLOUR_CORRECTED                 0xDCD2U
#define USB_MTP_OB_PROP_IMAGE_BIT_DEPTH                     0xDCD3U
#define USB_MTP_OB_PROP_FNUMBER                             0xDCD4U
#define USB_MTP_OB_PROP_EXPOSURE_TIME                       0xDCD5U
#define USB_MTP_OB_PROP_EXPOSURE_INDEX                      0xDCD6U
#define USB_MTP_OB_PROP_TOTAL_BITRATE                       0xDE91U
#define USB_MTP_OB_PROP_BITRATE_TYPE                        0xDE92U
#define USB_MTP_OB_PROP_SAMPLE_RATE                         0xDE93U
#define USB_MTP_OB_PROP_NUMBER_OF_CHANNELS                  0xDE94U
#define USB_MTP_OB_PROP_AUDIO_BITDEPTH                      0xDE95U
#define USB_MTP_OB_PROP_SCAN_TYPE                           0xDE97U
#define USB_MTP_OB_PROP_AUDIO_WAVE_CODEC                    0xDE99U
#define USB_MTP_OB_PROP_AUDIO_BITRATE                       0xDE9AU
#define USB_MTP_OB_PROP_VIDEO_FOURCC_CODEC                  0xDE9BU
#define USB_MTP_OB_PROP_VIDEO_BITRATE                       0xDE9CU
#define USB_MTP_OB_PROP_FRAMES_PER_THOUSAND_SECONDS         0xDE9DU
#define USB_MTP_OB_PROP_KEYFRAME_DISTANCE                   0xDE9EU
#define USB_MTP_OB_PROP_BUFFER_SIZE                         0xDE9FU
#define USB_MTP_OB_PROP_ENCODING_QUALITY                    0xDEA0U
#define USB_MTP_OB_PROP_ENCODING_PROFILE                    0xDEA1U
#define USB_MTP_OB_PROP_DISPLAY_NAME                        0xDCE0U
#define USB_MTP_OB_PROP_BODY_TEXT                           0xDCE1U
#define USB_MTP_OB_PROP_SUBJECT                             0xDCE2U
#define USB_MTP_OB_PROP_PRIORITY                            0xDCE3U
#define USB_MTP_OB_PROP_GIVEN_NAME                          0xDD00U
#define USB_MTP_OB_PROP_MIDDLE_NAMES                        0xDD01U
#define USB_MTP_OB_PROP_FAMILY_NAME                         0xDD02U
#define USB_MTP_OB_PROP_PREFIX                              0xDD03U
#define USB_MTP_OB_PROP_SUFFIX                              0xDD04U
#define USB_MTP_OB_PROP_PHONETIC_GIVEN_NAME                 0xDD05U
#define USB_MTP_OB_PROP_PHONETIC_FAMILY_NAME                0xDD06U
#define USB_MTP_OB_PROP_EMAIL_PRIMARY                       0xDD07U
#define USB_MTP_OB_PROP_EMAIL_PERSONAL_1                    0xDD08U
#define USB_MTP_OB_PROP_EMAIL_PERSONAL_2                    0xDD09U
#define USB_MTP_OB_PROP_EMAIL_BUSINESS_1                    0xDD0AU
#define USB_MTP_OB_PROP_EMAIL_BUSINESS_2                    0xDD0BU
#define USB_MTP_OB_PROP_EMAIL_OTHERS                        0xDD0CU
#define USB_MTP_OB_PROP_PHONE_NUMBER_PRIMARY                0xDD0DU
#define USB_MTP_OB_PROP_PHONE_NUMBER_PERSONAL               0xDD0EU
#define USB_MTP_OB_PROP_PHONE_NUMBER_PERSONAL_2             0xDD0FU
#define USB_MTP_OB_PROP_PHONE_NUMBER_BUSINESS               0xDD10U
#define USB_MTP_OB_PROP_PHONE_NUMBER_BUSINESS_2             0xDD11U
#define USB_MTP_OB_PROP_PHONE_NUMBER_MOBILE                 0xDD12U
#define USB_MTP_OB_PROP_PHONE_NUMBER_MOBILE_2               0xDD13U
#define USB_MTP_OB_PROP_FAX_NUMBER_PRIMARY                  0xDD14U
#define USB_MTP_OB_PROP_FAX_NUMBER_PERSONAL                 0xDD15U
#define USB_MTP_OB_PROP_FAX_NUMBER_BUSINESS                 0xDD16U
#define USB_MTP_OB_PROP_PAGER_NUMBER                        0xDD17U
#define USB_MTP_OB_PROP_PHONE_NUMBER_OTHERS                 0xDD18U
#define USB_MTP_OB_PROP_PRIMARY_WEB_ADDRESS                 0xDD19U
#define USB_MTP_OB_PROP_PERSONAL_WEB_ADDRESS                0xDD1AU
#define USB_MTP_OB_PROP_BUSINESS_WEB_ADDRESS                0xDD1BU
#define USB_MTP_OB_PROP_INSTANT_MESSENGER_ADDRESS           0xDD1CU
#define USB_MTP_OB_PROP_INSTANT_MESSENGER_ADDRESS_2         0xDD1DU
#define USB_MTP_OB_PROP_INSTANT_MESSENGER_ADDRESS_3         0xDD1EU
#define USB_MTP_OB_PROP_POSTAL_ADDRESS_PERSONAL_FULL        0xDD1FU
#define USB_MTP_OB_PROP_POSTAL_ADDRESS_PERSONAL_LINE_1      0xDD20U
#define USB_MTP_OB_PROP_POSTAL_ADDRESS_PERSONAL_LINE_2      0xDD21U
#define USB_MTP_OB_PROP_POSTAL_ADDRESS_PERSONAL_CITY        0xDD22U
#define USB_MTP_OB_PROP_POSTAL_ADDRESS_PERSONAL_REGION      0xDD23U
#define USB_MTP_OB_PROP_POSTAL_ADDRESS_PERSONAL_POSTAL_CODE 0xDD24U
#define USB_MTP_OB_PROP_POSTAL_ADDRESS_PERSONAL_COUNTRY     0xDD25U
#define USB_MTP_OB_PROP_POSTAL_ADDRESS_BUSINESS_FULL        0xDD26U
#define USB_MTP_OB_PROP_POSTAL_ADDRESS_BUSINESS_LINE_1      0xDD27U
#define USB_MTP_OB_PROP_POSTAL_ADDRESS_BUSINESS_LINE_2      0xDD28U
#define USB_MTP_OB_PROP_POSTAL_ADDRESS_BUSINESS_CITY        0xDD29U
#define USB_MTP_OB_PROP_POSTAL_ADDRESS_BUSINESS_REGION      0xDD2AU
#define USB_MTP_OB_PROP_POSTAL_ADDRESS_BUSINESS_POSTAL_CODE 0xDD2BU
#define USB_MTP_OB_PROP_POSTAL_ADDRESS_BUSINESS_COUNTRY     0xDD2CU
#define USB_MTP_OB_PROP_POSTAL_ADDRESS_OTHER_FULL           0xDD2DU
#define USB_MTP_OB_PROP_POSTAL_ADDRESS_OTHER_LINE_1         0xDD2EU
#define USB_MTP_OB_PROP_POSTAL_ADDRESS_OTHER_LINE_2         0xDD2FU
#define USB_MTP_OB_PROP_POSTAL_ADDRESS_OTHER_CITY           0xDD30U
#define USB_MTP_OB_PROP_POSTAL_ADDRESS_OTHER_REGION         0xDD31U
#define USB_MTP_OB_PROP_POSTAL_ADDRESS_OTHER_POSTAL_CODE    0xDD32U
#define USB_MTP_OB_PROP_POSTAL_ADDRESS_OTHER_COUNTRY        0xDD33U
#define USB_MTP_OB_PROP_ORGANIZATION_NAME                   0xDD34U
#define USB_MTP_OB_PROP_PHONETIC_ORGANIZATION_NAME          0xDD35U
#define USB_MTP_OB_PROP_ROLE                                0xDD36U
#define USB_MTP_OB_PROP_BIRTHDATE                           0xDD37U
#define USB_MTP_OB_PROP_MESSAGE_TO                          0xDD40U
#define USB_MTP_OB_PROP_MESSAGE_CC                          0xDD41U
#define USB_MTP_OB_PROP_MESSAGE_BCC                         0xDD42U
#define USB_MTP_OB_PROP_MESSAGE_READ                        0xDD43U
#define USB_MTP_OB_PROP_MESSAGE_RECEIVED_TIME               0xDD44U
#define USB_MTP_OB_PROP_MESSAGE_SENDER                      0xDD45U
#define USB_MTP_OB_PROP_ACT_BEGIN_TIME                      0xDD50U
#define USB_MTP_OB_PROP_ACT_END_TIME                        0xDD51U
#define USB_MTP_OB_PROP_ACT_LOCATION                        0xDD52U
#define USB_MTP_OB_PROP_ACT_REQUIRED_ATTENDEES              0xDD54U
#define USB_MTP_OB_PROP_ACT_OPTIONAL_ATTENDEES              0xDD55U
#define USB_MTP_OB_PROP_ACT_RESOURCES                       0xDD56U
#define USB_MTP_OB_PROP_ACT_ACCEPTED                        0xDD57U
#define USB_MTP_OB_PROP_OWNER                               0xDD5DU
#define USB_MTP_OB_PROP_EDITOR                              0xDD5EU
#define USB_MTP_OB_PROP_WEBMASTER                           0xDD5FU
#define USB_MTP_OB_PROP_URL_SOURCE                          0xDD60U
#define USB_MTP_OB_PROP_URL_DESTINATION                     0xDD61U
#define USB_MTP_OB_PROP_TIME_BOOKMARK                       0xDD62U
#define USB_MTP_OB_PROP_OBJECT_BOOKMARK                     0xDD63U
#define USB_MTP_OB_PROP_BYTE_BOOKMARK                       0xDD64U
#define USB_MTP_OB_PROP_LAST_BUILD_DATE                     0xDD70U
#define USB_MTP_OB_PROP_TIME_TO_LIVE                        0xDD71U
#define USB_MTP_OB_PROP_MEDIA_GUID                          0xDD72U

/*  MTP event codes*/
#define USB_MTP_EVENT_UNDEFINED               0x4000U
#define USB_MTP_EVENT_CANCELTRANSACTION       0x4001U
#define USB_MTP_EVENT_OBJECTADDED             0x4002U
#define USB_MTP_EVENT_OBJECTREMOVED           0x4003U
#define USB_MTP_EVENT_STOREADDED              0x4004U
#define USB_MTP_EVENT_STOREREMOVED            0x4005U
#define USB_MTP_EVENT_DEVICEPROPCHANGED       0x4006U
#define USB_MTP_EVENT_OBJECTINFOCHANGED       0x4007U
#define USB_MTP_EVENT_DEVICEINFOCHANGED       0x4008U
#define USB_MTP_EVENT_REQUESTOBJECTTRANSFER   0x4009U
#define USB_MTP_EVENT_STOREFULL               0x400AU
#define USB_MTP_EVENT_DEVICERESET             0x400BU
#define USB_MTP_EVENT_STORAGEINFOCHANGED      0x400CU
#define USB_MTP_EVENT_CAPTURECOMPLETE         0x400DU
#define USB_MTP_EVENT_UNREPORTEDSTATUS        0x400EU
#define USB_MTP_EVENT_OBJECTPROPCHANGED       0xC801U
#define USB_MTP_EVENT_OBJECTPROPDESCCHANGED   0xC802U
#define USB_MTP_EVENT_OBJECTREFERENCESCHANGED 0xC803U

/*
 * MTP Class specification Revision 1.1
 * Appendix D. Operations
 */

/* Operations code */
#define USB_MTP_OP_GET_DEVICE_INFO            0x1001U
#define USB_MTP_OP_OPEN_SESSION               0x1002U
#define USB_MTP_OP_CLOSE_SESSION              0x1003U
#define USB_MTP_OP_GET_STORAGE_IDS            0x1004U
#define USB_MTP_OP_GET_STORAGE_INFO           0x1005U
#define USB_MTP_OP_GET_NUM_OBJECTS            0x1006U
#define USB_MTP_OP_GET_OBJECT_HANDLES         0x1007U
#define USB_MTP_OP_GET_OBJECT_INFO            0x1008U
#define USB_MTP_OP_GET_OBJECT                 0x1009U
#define USB_MTP_OP_GET_THUMB                  0x100AU
#define USB_MTP_OP_DELETE_OBJECT              0x100BU
#define USB_MTP_OP_SEND_OBJECT_INFO           0x100CU
#define USB_MTP_OP_SEND_OBJECT                0x100DU
#define USB_MTP_OP_FORMAT_STORE               0x100FU
#define USB_MTP_OP_RESET_DEVICE               0x1010U
#define USB_MTP_OP_GET_DEVICE_PROP_DESC       0x1014U
#define USB_MTP_OP_GET_DEVICE_PROP_VALUE      0x1015U
#define USB_MTP_OP_SET_DEVICE_PROP_VALUE      0x1016U
#define USB_MTP_OP_RESET_DEVICE_PROP_VALUE    0x1017U
#define USB_MTP_OP_TERMINATE_OPEN_CAPTURE     0x1018U
#define USB_MTP_OP_MOVE_OBJECT                0x1019U
#define USB_MTP_OP_COPY_OBJECT                0x101AU
#define USB_MTP_OP_GET_PARTIAL_OBJECT         0x101BU
#define USB_MTP_OP_INITIATE_OPEN_CAPTURE      0x101CU
#define USB_MTP_OP_GET_OBJECT_PROPS_SUPPORTED 0x9801U
#define USB_MTP_OP_GET_OBJECT_PROP_DESC       0x9802U
#define USB_MTP_OP_GET_OBJECT_PROP_VALUE      0x9803U
#define USB_MTP_OP_SET_OBJECT_PROP_VALUE      0x9804U
#define USB_MTP_OP_GET_OBJECT_PROPLIST        0x9805U
#define USB_MTP_OP_GET_OBJECT_PROP_REFERENCES 0x9810U
#define USB_MTP_OP_GETSERVICEIDS              0x9301U
#define USB_MTP_OP_GETSERVICEINFO             0x9302U
#define USB_MTP_OP_GETSERVICECAPABILITIES     0x9303U
#define USB_MTP_OP_GETSERVICEPROPDESC         0x9304U

/*
 * MTP Class specification Revision 1.1
 * Appendix C. Device Properties
 */

/* MTP device properties code*/
#define USB_MTP_DEV_PROP_UNDEFINED                      0x5000U
#define USB_MTP_DEV_PROP_BATTERY_LEVEL                  0x5001U
#define USB_MTP_DEV_PROP_FUNCTIONAL_MODE                0x5002U
#define USB_MTP_DEV_PROP_IMAGE_SIZE                     0x5003U
#define USB_MTP_DEV_PROP_COMPRESSION_SETTING            0x5004U
#define USB_MTP_DEV_PROP_WHITE_BALANCE                  0x5005U
#define USB_MTP_DEV_PROP_RGB_GAIN                       0x5006U
#define USB_MTP_DEV_PROP_F_NUMBER                       0x5007U
#define USB_MTP_DEV_PROP_FOCAL_LENGTH                   0x5008U
#define USB_MTP_DEV_PROP_FOCUS_DISTANCE                 0x5009U
#define USB_MTP_DEV_PROP_FOCUS_MODE                     0x500AU
#define USB_MTP_DEV_PROP_EXPOSURE_METERING_MODE         0x500BU
#define USB_MTP_DEV_PROP_FLASH_MODE                     0x500CU
#define USB_MTP_DEV_PROP_EXPOSURE_TIME                  0x500DU
#define USB_MTP_DEV_PROP_EXPOSURE_PROGRAM_MODE          0x500EU
#define USB_MTP_DEV_PROP_EXPOSURE_INDEX                 0x500FU
#define USB_MTP_DEV_PROP_EXPOSURE_BIAS_COMPENSATION     0x5010U
#define USB_MTP_DEV_PROP_DATETIME                       0x5011U
#define USB_MTP_DEV_PROP_CAPTURE_DELAY                  0x5012U
#define USB_MTP_DEV_PROP_STILL_CAPTURE_MODE             0x5013U
#define USB_MTP_DEV_PROP_CONTRAST                       0x5014U
#define USB_MTP_DEV_PROP_SHARPNESS                      0x5015U
#define USB_MTP_DEV_PROP_DIGITAL_ZOOM                   0x5016U
#define USB_MTP_DEV_PROP_EFFECT_MODE                    0x5017U
#define USB_MTP_DEV_PROP_BURST_NUMBER                   0x5018U
#define USB_MTP_DEV_PROP_BURST_INTERVAL                 0x5019U
#define USB_MTP_DEV_PROP_TIMELAPSE_NUMBER               0x501AU
#define USB_MTP_DEV_PROP_TIMELAPSE_INTERVAL             0x501BU
#define USB_MTP_DEV_PROP_FOCUS_METERING_MODE            0x501CU
#define USB_MTP_DEV_PROP_UPLOAD_URL                     0x501DU
#define USB_MTP_DEV_PROP_ARTIST                         0x501EU
#define USB_MTP_DEV_PROP_COPYRIGHT_INFO                 0x501FU
#define USB_MTP_DEV_PROP_SYNCHRONIZATION_PARTNER        0xD401U
#define USB_MTP_DEV_PROP_DEVICE_FRIENDLY_NAME           0xD402U
#define USB_MTP_DEV_PROP_VOLUME                         0xD403U
#define USB_MTP_DEV_PROP_SUPPORTEDFORMATSORDERED        0xD404U
#define USB_MTP_DEV_PROP_DEVICEICON                     0xD405U
#define USB_MTP_DEV_PROP_PLAYBACK_RATE                  0xD410U
#define USB_MTP_DEV_PROP_PLAYBACK_OBJECT                0xD411U
#define USB_MTP_DEV_PROP_PLAYBACK_CONTAINER             0xD412U
#define USB_MTP_DEV_PROP_SESSION_INITIATOR_VERSION_INFO 0xD406U
#define USB_MTP_DEV_PROP_PERCEIVED_DEVICE_TYPE          0xD407U

/* Container Types */
#define USB_MTP_CONT_TYPE_UNDEFINED 0U
#define USB_MTP_CONT_TYPE_COMMAND   1U
#define USB_MTP_CONT_TYPE_DATA      2U
#define USB_MTP_CONT_TYPE_RESPONSE  3U
#define USB_MTP_CONT_TYPE_EVENT     4U

#ifndef USB_MTP_STORAGE_ID
#define USB_MTP_STORAGE_ID 0x00010001U /* SD card is inserted*/
#endif                             /* USB_MTP_STORAGE_ID */

#define USB_MTP_NBR_STORAGE_ID         1U
#define FREE_SPACE_IN_OBJ_NOT_USED 0xFFFFFFFFU

/* MTP storage type */
#define USB_MTP_STORAGE_UNDEFINED     0U
#define USB_MTP_STORAGE_FIXED_ROM     0x0001U
#define USB_MTP_STORAGE_REMOVABLE_ROM 0x0002U
#define USB_MTP_STORAGE_FIXED_RAM     0x0003U
#define USB_MTP_STORAGE_REMOVABLE_RAM 0x0004U

/* MTP file system type */
#define USB_MTP_FILESYSTEM_UNDEFINED        0U
#define USB_MTP_FILESYSTEM_GENERIC_FLAT     0x0001U
#define USB_MTP_FILESYSTEM_GENERIC_HIERARCH 0x0002U
#define USB_MTP_FILESYSTEM_DCF              0x0003U

/* MTP access capability */
#define USB_MTP_ACCESS_CAP_RW             0U /* read write */
#define USB_MTP_ACCESS_CAP_RO_WITHOUT_DEL 0x0001U
#define USB_MTP_ACCESS_CAP_RO_WITH_DEL    0x0002U

/* MTP standard data types supported */
#define USB_MTP_DATATYPE_INT8    0x0001U
#define USB_MTP_DATATYPE_UINT8   0x0002U
#define USB_MTP_DATATYPE_INT16   0x0003U
#define USB_MTP_DATATYPE_UINT16  0x0004U
#define USB_MTP_DATATYPE_INT32   0x0005U
#define USB_MTP_DATATYPE_UINT32  0x0006U
#define USB_MTP_DATATYPE_INT64   0x0007U
#define USB_MTP_DATATYPE_UINT64  0x0008U
#define USB_MTP_DATATYPE_UINT128 0x000AU
#define USB_MTP_DATATYPE_STR     0xFFFFU

/* MTP reading only or reading/writing */
#define USB_MTP_PROP_GET     0x00U
#define USB_MTP_PROP_GET_SET 0x01U

#endif /* USB_MTP_H */
