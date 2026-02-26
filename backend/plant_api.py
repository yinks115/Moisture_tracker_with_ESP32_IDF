from flask import Flask
from flask_restx import Api # will be used for the swagger

# from routes.say_hello import say_hello_ns
# from routes.submit_reading import submit_reading_ns
# from routes.get_readings_by_plant_name import get_readings_by_plant_name_ns

app = Flask(__name__)


api = Api(app, version='1.0', title='Plants API', description='Try the API here', doc='/api/')

# api.add_namespace(say_hello_ns)
# api.add_namespace(submit_reading_ns)
# api.add_namespace(get_readings_by_plant_name_ns)