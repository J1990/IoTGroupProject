import requests, json 
import time
from datetime import datetime
import csv
from crontab import CronTab


if __name__ == '__main__':
    
    api_key = "2f30e9abf594e715af4449414b055800"
    base_url = "http://api.openweathermap.org/data/2.5/weather?"
    city_name = "Dublin,IE"
    weather_api_url = base_url + "appid=" + api_key + "&q=" + city_name + "&units=metric"
    response = requests.get(weather_api_url) 

    x = response.json() 

    # Parsing the response to get weather attributes
    if x["cod"] != "404": 

        y = x["main"] 

        current_temperature = y["temp"] 
        
        max_temp = y['temp_max']
        
        min_temp = y['temp_min']
        
        feels_like = y['feels_like']

        current_pressure = y["pressure"] 

        current_humidiy = y["humidity"] 

        z = x["weather"] 

        weather_description = z[0]["description"] 

        sun_times = x['sys']

        sunrise = sun_times['sunrise']
        sunset = sun_times['sunset']

        # Writing the data to a csv file

        with open('weather_data.csv', mode='w') as file:
            writer = csv.writer(file, delimiter=',', quotechar='"', quoting=csv.QUOTE_MINIMAL)
            writer.writerow(['updated_time', 'temperature', 'sunrise', 'sunset', 'description'])
            writer.writerow([datetime.now().strftime("%m/%d/%Y, %H:%M:%S"), current_temperature, time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(sunrise)),
                             time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(sunset)), weather_description])
        
        sunset_cron = time.strftime('%M', time.localtime(sunset)) + ' ' + time.strftime('%H', time.localtime(sunset)) + ' * * *'
        sunrise_cron = time.strftime('%M', time.localtime(sunrise)) + ' ' + time.strftime('%H', time.localtime(sunrise)) + ' * * *'
        
        my_cron = CronTab(user='iot_weather_info')
        
        for job in my_cron:
            if job.comment == 'sunrise_job':
                my_cron.remove(job)
                my_cron.write()
            if job.comment == 'sunset_job':
                my_cron.remove(job)
                my_cron.write()
        
        job_sunrise = my_cron.new(command='python3 publish_sunrise.py', comment='sunrise_job')
        job_sunrise.setall(sunrise_cron)
        job_sunset = my_cron.new(command='python3 publish_sunset.py', comment='sunset_job')
        job_sunset.setall(sunset_cron)
        
        my_cron.write()
        
        with open('log_file.txt', mode='w') as file:
            file.write('Updated at ' + datetime.now().strftime("%m/%d/%Y, %H:%M:%S") + ' : Fetching Weather API Successful \n')
        
    else:
        print("Could not get response from API")
