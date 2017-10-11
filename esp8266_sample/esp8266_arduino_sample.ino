//requirement
https://github.com/kakopappa/arduino-esp8266-aes-lib

String DeCrypt(String PAIRCODE, String data){
	
	Serial.println("DeCrypt payload");
		
	String decrypted = "";
	String IV_base64 = "";
	
	//Serial.println("> "+data);
	//if(data.length()>1024)Serial.println("Payload too large");
		
	//Serial.println("D000");
	byte paircode[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	PAIRCODE.getBytes(paircode, 16);
		
	boolean open_array=false;
	boolean open_string=false;
	boolean next_member=false;
	String member="";
	
	boolean first_member=true;	
    	//char data_decoded[300];
	//char iv_decoded[200];
	
	for(int i=0;i<data.length();i++){
		
		if(next_member){
			
			if(first_member){
				
				//IV
				IV_base64 = member;
				
				first_member=false;
				
			}else{
				
				//DATA
				String b64data = member;
				
				char iv_decoded[64];
				char data_decoded[64];
				
				{				
				char temp[64];
				IV_base64.toCharArray(temp, 64);
				
				base64_decode(iv_decoded, temp, IV_base64.length());
				}
				
				int b64len=0;
				{				
				char temp[64];
				for(int i=0;i<64;i++)data_decoded[i]='\0';
				
				b64data.toCharArray(temp, 64);				
				b64len = base64_decode(data_decoded, temp, b64data.length());
				}
				
				//DECRYPT
				String member_decrypted="";
				{				
				AES aes;
				byte out[64];
				for(int i=0;i<64;i++)out[i]=0;
				
				//aes.do_aes_decrypt((byte *)data_decoded, b64data.length(), out, paircode, 128, (byte *)iv_decoded);
				aes.do_aes_decrypt((byte *)data_decoded, b64len, out, paircode, 128, (byte *)iv_decoded);
				aes.clean();
								
				char temp[b64data.length()];				
				for(int i=0;i<b64data.length();i++)temp[i]='\0';
								
				//int len = base64_decode(temp, (char *)out, aes.get_size());
				int len = base64_decode(temp, (char *)out, b64data.length());
								
				member_decrypted = String(temp);
				//Serial.print(len);Serial.print(">");
				//Serial.println(member_decrypted);
				//decrypted = decrypted+String(temp);//(char *)temp);				
				}
				decrypted += member_decrypted;
				
				}
			
			member="";
			next_member=false;
			}
			
		boolean dato=true;
		if(data.substring(i,i+1)=="["){
			
			open_array=true;
			dato=false;
			}
			
		if(data.substring(i,i+1)=="]"){
			
			open_array=false;
			dato=false;
			}
			
		if(data.substring(i,i+1)=="\""){
						
			if(open_string)next_member=true;
			
			open_string=!open_string;
			dato=false;
			}
		
		if(data.substring(i,i+1)==","){
						
			dato=false;
			}		
		
		if(dato){
			
			member+=data.substring(i,i+1);
			}
		}
	
	return decrypted;
	}
	

//IV
uint8_t getrnd() {
    uint8_t really_random = *(volatile uint8_t *)0x3FF20E44;
    return really_random;
	}

// Generate a random initialization vector
void gen_iv(byte  *iv) {
    for (int i = 0 ; i < N_BLOCK ; i++ ) {
        iv[i]= (byte) getrnd();
		}
	}

String CryptString(String subdata, byte  *paircode, byte  *iv){
		
	//AES aes;
	char b64data[64];
		
	//Serial.println(ESP.getFreeHeap());
	//---new	
	for(int i=0;i<64;i++)b64data[i]='\0';
		
	char input[16];	
	
	for(int i=0;i<16;i++)input[i]='\0';	
	
	sprintf(input,"%s",subdata.c_str());	
	
	int b64len = base64_encode(b64data, input, 16);
			
	{
	byte cipher[64];	
	
	AES aes;
	//for(int k=0;k<300;k++)cipher[k]=0x00;	
	aes.do_aes_encrypt((byte *)b64data, b64len, cipher, paircode, 128, iv);
		
	//Serial.println(ESP.getFreeHeap());	
	for(int i=0;i<64;i++)b64data[i]='\0';
		
	//Serial.println( aes.get_size());
	base64_encode(b64data, (char *)cipher, aes.get_size());
	
	aes.clean();
	}
		
	//Serial.println("CRYPTED="+String(b64data));
	return String(b64data);	
	}
	
String Crypt(String PAIRCODE, String data){

	String crypted = "[";
	//StaticJsonBuffer<512> jsonBuffer;
	//JsonArray& array = jsonBuffer.createArray();
		
	byte iv[N_BLOCK], iv1[N_BLOCK];
	
	Serial.println("Crypt payload");
	if(data.length()>512)Serial.println("Payload too large");
	
	byte paircode[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	PAIRCODE.getBytes(paircode, 16);
	
	gen_iv(iv);	
	
	{
	char b64data[128];
	    
	for(int i=0;i<128;i++)b64data[i]='\0';
	base64_encode(b64data, (char *)iv, N_BLOCK);
	String IV_base64 = String(b64data);
	
	//array.add( IV_base64 );
	crypted+="\""+IV_base64+"\"";
	}
	
	int i=0;
	for(i=0;data.length()>=16 && i<(data.length()-16);i+=16){
		
		ESP.wdtFeed();
		for(int k=0;k<N_BLOCK;k++){
			iv1[k]=iv[k];			
			}
		
		String subdata = data.substring(i, i+16);
		//Serial.println("Crypt ["+subdata+"]");
		
		//---new
		//array.add( CryptString(subdata, paircode, iv1) );
		String tmp=",\""+CryptString(subdata, paircode, iv1)+"\"";
		crypted+=tmp;
		}
	
	if( (data.length() - i)>0){
				
		ESP.wdtFeed();
		for(int k=0;k<N_BLOCK;k++){
			iv1[k]=iv[k];			
			}
		
		String subdata = data.substring(i);
		
		int remain = 16 - (data.length() - i);
		for(int f=0;f<remain;f++)subdata+=" ";
		//Serial.println("Crypt ["+subdata+"]");
		
		//---new
		//array.add( CryptString(subdata, paircode, iv1) );
		String tmp=",\""+CryptString(subdata, paircode, iv1)+"\"";
		crypted+=tmp;
		}
						
	//array.printTo(crypted);
	crypted+="]";
	
	return crypted;
	}	