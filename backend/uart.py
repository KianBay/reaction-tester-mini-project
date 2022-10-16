import serial
from time import sleep, time



class UART: #UART class
    def __init__(self, ser):
        self.ser = serial.Serial ("/dev/ttyS0", 9600)

    def send(self, data): #send data
        self.ser.write(data.encode())

    def receive(self): #receive data
        data = []
        i = 0
        while True:
            initialChar = self.ser.read() 
            sleep(0.03)
            
            if initialChar == b'D': #Still seem to only fetch the first 2 bytes?

                data_left = self.ser.inWaiting()
                received_data = self.ser.read(data_left)
                frame = {"score": int.from_bytes(received_data, byteorder='big'), "timestamp": time()}
                data.append(frame) 
                
                continue
            elif initialChar == b'E':
                return {"error": "Error Error Error"}
            elif initialChar == b'R':
                return data

    def receive_scores(self):
        data = self.receive()
        #print(f"Scores: {data}, Length: {len(data)}")
        print(data)
        return data

 

    def close(self): #close connection
        self.ser.close()


if __name__ == "__main__":
    #ser = UART(serial.Serial("/dev/ttyS0", 9600))
    #ser.receive()
    ser = UART(serial.Serial ("/dev/ttyS0", 9600))    #Open port with baud rate
    ser.receive_scores()

        #print('Decoded Data: ', received_data.decode('utf-8'))              #print received data 
    
        