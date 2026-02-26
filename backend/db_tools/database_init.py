import os
import sqlite3
from env_variables import DB_PATH

def create_db_tables():

    # checks if db was already created
    path = os.path.join(os.getcwd(), DB_PATH) # gets db path
    if os.path.exists(path):
        print(f"Warning! The Database already exists")
        return -1
    
    # creates the db folder before creating db file
    parent_dir = os.path.dirname(path)
    if not os.path.exists(parent_dir):
        os.makedirs(parent_dir)
        print(f"Created DB folder")

    # creates the actual db
    try:
        conn = sqlite3.connect(path) # creates the db
        cursor = conn.cursor() # lets us interact with the db

        cursor.execute(
            '''
            CREATE TABLE IF NOT EXISTS plants_reading(
            id INTEGER PRIMARY KEY,
            plant_name TEXT,
            moisture REAL,
            server_timestamp TIMESTAMP DEFAULT CURRENT_TIME
            )

            '''
        )
        conn.commit()

    except sqlite3.DatabaseError as err:
        print(f"Error encountered while creating the database: {err}")
        return -1
    finally:
        conn.close()

    return 0