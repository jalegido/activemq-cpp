/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "OpenwireStringSupport.h"

#include <activemq/util/Integer.h>

using namespace activemq;
using namespace activemq::io;
using namespace activemq::util;
using namespace activemq::exceptions;
using namespace activemq::connector;
using namespace activemq::connector::openwire;
using namespace activemq::connector::openwire::utils;

////////////////////////////////////////////////////////////////////////////////
std::string OpenwireStringSupport::readString( io::DataInputStream& dataIn )
    throw ( io::IOException ) {
        
    try {

        short utflen = dataIn.readShort();
        
        if( utflen > -1 )
        {
            // Let the stream get us all that data.
            std::vector<unsigned char> value;
            value.resize( utflen );
            dataIn.readFully( value );            

            int c, char2, char3;
            int count = 0;
            
            while( count < utflen )
            {
                c = value[count] & 0xff;
                switch( c >> 4 )
                {
                    case 0:
                    case 1:
                    case 2:
                    case 3:
                    case 4:
                    case 5:
                    case 6:
                    case 7:
                        /* 0xxxxxxx */
                        count++;
//                        str.Append((char) c);
                        break;
                    case 12:
                    case 13:
//                        /* 110x xxxx 10xx xxxx */
//                        count += 2;
//                        if (count > utflen)
//                        {
//                            throw CreateDataFormatException();
//                        }
//                        char2 = bytearr[count - 1];
//                        if ((char2 & 0xC0) != 0x80)
//                        {
//                            throw CreateDataFormatException();
//                        }
//                        str.Append((char) (((c & 0x1F) << 6) | (char2 & 0x3F)));
//                        break;
                    case 14:
//                        /* 1110 xxxx 10xx xxxx 10xx xxxx */
//                        count += 3;
//                        if (count > utflen)
//                        {
//                            throw CreateDataFormatException();
//                        }
//                        char2 = bytearr[count - 2];
//                        char3 = bytearr[count - 1];
//                        if (((char2 & 0xC0) != 0x80) || ((char3 & 0xC0) != 0x80))
//                        {
//                            throw CreateDataFormatException();
//                        }
//                        str.Append((char) (((c & 0x0F) << 12) | ((char2 & 0x3F) << 6) | ((char3 & 0x3F) << 0)));
//                        break;
                    default :
                    {
                        /* 10xx xxxx, 1111 xxxx */
                        throw IOException(
                            __FILE__,
                            __LINE__,
                            "OpenwireStringSupport::readString - Encoding not supported" );
                    }
                }
            }

            // C++ strings need a NULL terminator
            value.push_back( '\0' );

            // Let the Compiler give us a string.
            return reinterpret_cast<const char*>( &value[0] );
        }

        return "";
    }
    AMQ_CATCH_RETHROW( ActiveMQException )
    AMQ_CATCHALL_THROW( ActiveMQException )        
}

////////////////////////////////////////////////////////////////////////////////
void OpenwireStringSupport::writeString( io::DataOutputStream& dataOut, 
                                         const std::string* str )
                                            throw ( io::IOException ) {

    try {

        if( str != NULL )
        {
            if( str->size() > 65536 ) {
                throw IOException(
                    __FILE__,
                    __LINE__,
                    ( std::string( "OpenwireStringSupport::writeString - Cannot marshall " ) + 
                    "string longer than: 65536 characters, supplied steing was: " +
                    Integer::toString( str->size() ) + " characters long." ).c_str() );
            }
            
            short strlen = (short)str->size();
            short utflen = 0;
            int c, count = 0;
            
            std::string::const_iterator iter = str->begin();
            
            for(; iter != str->end(); ++iter ) {
                c = *iter;
                if( (c >= 0x0001) && (c <= 0x007F) ) {
                    utflen++;
                } else if( c > 0x07FF ) {
                    utflen += 3;
                } else {
                    utflen += 2;
                }
            }
            
            dataOut.writeShort( utflen );
            std::vector<unsigned char> byteArr;
            byteArr.resize( utflen );

            for( iter = str->begin(); iter != str->end(); ++iter ) {

                c = *iter;
                if( (c >= 0x0001) && (c <= 0x007F) ) {
                    byteArr[count++] = (unsigned char)c;
                } else if ( c > 0x07FF ) {
                    byteArr[count++] = (unsigned char)( 0xE0 | ( (c >> 12) & 0x0F) );
                    byteArr[count++] = (unsigned char)( 0x80 | ( (c >> 6) & 0x3F) );
                    byteArr[count++] = (unsigned char)( 0x80 | ( (c >> 0) & 0x3F) );
                } else {
                    byteArr[count++] = (unsigned char)( 0xC0 | ( (c >> 6) & 0x1F) );
                    byteArr[count++] = (unsigned char)( 0x80 | ( (c >> 0) & 0x3F) );
                }
            }

            dataOut.write( byteArr );
        }
        else
        {
            dataOut.writeShort( (short)-1 );
        }
    }
    AMQ_CATCH_RETHROW( ActiveMQException )
    AMQ_CATCHALL_THROW( ActiveMQException )        
} 
