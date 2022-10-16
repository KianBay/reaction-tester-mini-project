from uart import UART
import serial
from time import sleep, time

from fastapi import FastAPI
# pi@leSPy
app = FastAPI() #python -m uvicorn main:app --host 0.0.0.0 --port 4848 --reload 


@app.get("/")
async def root():
    return {"message": "Hello World"}

@app.get("/play/{attempts}")
async def play(attempts: int):
    ser = UART(serial.Serial ("/dev/ttyS0", 9600))
    if attempts > 9 | attempts == 0:
        return {"error": f"Invalid number of tries: {attempts}"}

    #ser.send('c')
    #ser.send(attempts)
    data = ser.receive()
    print(f'Tries sent: {attempts}')
    return {data} # Redirect somewhere cool after game is played? 
        





# if __name__ == "__main__":

    