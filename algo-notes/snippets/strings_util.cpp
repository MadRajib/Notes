#include <iostream>
#include <string>
#include <vector>

using namespace std;

void process(char *bar,int e){
    // use N here
  if(e-0 +1 != 3){
    cout<<"No";
    return;
  }
  
  if(bar[2] > bar[1] && bar[2] > bar[0]){
    cout<<"yes";
  }else{
    cout<<"no";
  }
  cout<<endl;

}

void raw_split(string s,char delim = ' '){
  const int sz = s.length();  

  char buffer[sz];

  for(int i = 0,j=0;i <=sz;i++){

    if( i == sz || delim == s[i]){

      // process(buffer,j-1);
      for(int k = 0;k<j;k++){
            cout<<buffer[k];
      }
      cout<<endl;
      
      j = 0;
        
    }else buffer[j++] = s[i];
      

  }
}


int main(){
  string s = "112 12 2";
  string s1 = "112.12.2";
  raw_split(s);
  raw_split(s1,'.');
}