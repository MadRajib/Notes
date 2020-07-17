#include <iostream>
using namespace std;
namespace mylib{

  // min template
  template  <class  T>
  T min(const T &a ,const T &b ){
    return a < b ? a:b;
  }

  //max template
  template <class T>
  T max(const T &a,const T &b){
    return a < b?b:a;
  }

  //pair
  template <class T, class U>
  class pair{
    public:
    T first;
    U second;

    pair(T a,U b){
      this->first = a;
      this->second = b;
    }
  };


  //stack
  template <class T>
  class stack{
    private:
    // to hold data
      struct stackNode{        
        T data;
        stackNode *next;
        stackNode(T data){
          this->data = data;
        }
      };

      stackNode *_top = nullptr;

    public:
      bool empty(){
        return _top == nullptr;
      }
      T top(){
        if (_top == nullptr) { throw 20;}
        return _top->data;
      }


      void push(T item){
        stackNode *t = new stackNode(item);
        t->next = _top;
        _top = t;
      }


      void pop(){
        if(_top == nullptr) { throw 20;}
        stackNode* temp = _top;
        _top = _top->next;
        delete temp;
      }
  };


  //stack
  template <class T>
  class queue{
    private:
    // to hold data
      struct queueNode{        
        T data;
        queueNode *next;
        queueNode(T data){
          this->data = data;
        }
      };

      queueNode *_front = nullptr;
      queueNode *_rear = _front;

    public:
      bool empty(){
        return _front == nullptr;
      }
      T front(){
        if (_front == nullptr) { throw 20;}
        return _front->data;
      }


      void push(T item){
        queueNode *t = new queueNode(item);
        if(_rear != nullptr) _rear->next = t;
        _rear = t;
        if(_front == nullptr) _front = t;
      }


      void pop(){
        if(_front == nullptr) { throw 20;}
        queueNode* temp = _front;
        _front = _front->next;
        if(_front == nullptr) _rear = _front;
        delete temp;
      }
  };



  // void heapify(int *arr,int idx,int mx){
  //     int left = 2*idx+1;
  //     int right = 2*idx +2;
  //     int largest = idx;
  //     if(left < mx && arr[left]>arr[idx]) largest = left;
  //     if(right < mx && arr[right]>arr[largest]) largest = right;
  //     if(largest!=idx){
  //         int tmp = arr[largest];
  //         arr[largest] = arr[idx];
  //         arr[idx] = tmp;
  //         heapify(arr,largest,mx);
  //     }
  // }

  // void build_heap(int *arr,int mx){
  //     for(int i =(mx/2) -1;i>=0;--i){
  //         heapify(arr,i,mx);
  //     }
  // }

  // void heap_sort(int *arr,int sz){
  //   build_heap(arr,sz);
  //   for(int i=sz-1;i>=1;i--){
  //       int tmp = arr[i];
  //       arr[i] = arr[0];
  //       arr[0] = tmp;
  //       heapify(arr,0,i);
  //   }
  // }

  

  // without comparator
  template <typename Iterator>
  void heapify(Iterator begin,Iterator end,Iterator parent){
      auto left = begin + 2*(parent - begin)+ 1;
      auto right = begin + 2*(parent - begin) + 2;
      auto largest = parent;

      if(left < end && *left < *parent) largest = left;
      if(right < end && *right < *largest) largest = right;

      if(largest!=parent){
          auto tmp = *largest;
          *largest = *parent;
          *parent = tmp;
          
          heapify(begin,end,largest);
      }
  }

  template <typename Iterator>
  void make_heap(Iterator begin, Iterator end){
    int sz = end -begin;
    for( int i = (sz/2) -1;i>=0;--i){
      heapify(begin,end,begin+i);
    }
  }

  template <typename Iterator>
   void sort(Iterator begin,Iterator end){
    mylib::make_heap(begin,end);
    int sz = end - begin;
    for(int i=sz-1;i>=1;i--){
        auto tmp = *(begin+i);
        *(begin+i) = *begin;
        *begin = tmp;
        heapify(begin,begin+i,begin);
    }
  }


  //with comparator
  template <typename Iterator,class Compare>
  void heapify(Iterator begin,Iterator end,Iterator parent,Compare comp){
      auto left = begin + 2*(parent - begin)+ 1;
      auto right = begin + 2*(parent - begin) + 2;
      auto largest = parent;

      if(left < end && comp(*left,*parent)) largest = left;
      if(right < end && comp(*right,*largest)) largest = right;

      if(largest!=parent){
          auto tmp = *largest;
          *largest = *parent;
          *parent = tmp;
          
          heapify(begin,end,largest,comp);
      }
  }
  
  template <typename Iterator,class Compare>
  void make_heap(Iterator begin, Iterator end,Compare comp){
    int sz = end -begin;
    for( int i = (sz/2) -1;i>=0;--i){
      heapify(begin,end,begin+i,comp);
    }
  }

  template <typename Iterator, class Compare>
   void sort(Iterator begin,Iterator end,Compare comp){
    mylib::make_heap(begin,end,comp);
    int sz = end - begin;
    for(int i=sz-1;i>=1;i--){
        auto tmp = *(begin+i);
        *(begin+i) = *begin;
        *begin = tmp;
        heapify(begin,begin+i,begin,comp);
    }
  }

}

bool comp(int a,int b){
    if(a > b) return true;
    return false;
  }





int main(){
  // cout<<mylib::min(10.8,2.3)<<endl;
  // cout<<mylib::max(10,2)<<endl;

  // mylib::pair<int,int> p1(1,2);

  // cout<<p1.second<<endl;

  // mylib::stack<int> stk;
  // stk.push(2);
  // stk.push(9);
  
  // cout<< stk.top() <<endl;
  // stk.pop();
  // cout<< stk.top() <<endl;
  // stk.pop();
  // cout<< stk.top() <<endl;
  
  // mylib::queue<int> queue;

  // queue.push(1);
  // queue.push(2);
  // queue.push(3);

  // cout<< queue.front() <<endl;
  // queue.pop();
  // cout<<queue.front()<<endl;
  // queue.pop();
  // cout<<queue.front()<<endl;
  // queue.push(5);
  // queue.pop();
  // queue.pop();
  // cout<<queue.front()<<endl;



  // mylib::stack<pair<int ,int>> stk1;
  // stk1.push({1,2});
  // stk1.push({3,4});
  
  // cout<<"first :"<<stk1.top().first << "second: " << stk1.top().second <<endl;
  // stk1.pop();
  // cout<<"first :"<<stk1.top().first << "second: " <<stk1.top().second <<endl;
  // stk1.pop();
  
  // vector<int> v {7,1,10,4,6,9,2,11,3,5,12,8};
  
  // mylib::make_heap(v.begin(), v.end());
  
  // mylib::sort(v.begin(),v.end());
  // for(auto x:v) cout<<x<<" ";
  // cout<<endl;

  // mylib::sort(v.begin(),v.end(),comp);

  // for(auto x:v) cout<<x<<" ";



}