#pragma once

class msBuffer
{
    private:
    char *data_; // ownerLoop_ owns this event
    unsigned int capacity_;
    unsigned int dataLength_;

    public:
    msBuffer(unsigned int cap);
    ~msBuffer();

    char * data() const {
        return data_;
    }

    unsigned int capacity() const {
        return capacity_;
    }

    unsigned int dataLength() const {
        return dataLength_;
    }

    void set_dataLength(unsigned int newLen) {
        dataLength_ = newLen;
    }


    void clear();
    int resize(unsigned int len);

};
