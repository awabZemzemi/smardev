from flask import Flask, render_template, request, jsonify, redirect, url_for
from datetime import datetime, timedelta

app = Flask(__name__)

# In-memory data
authorized_tags = {
    "32296730": "AWAB"
}

user_logs = {}
blocked_users = set()
current_users = {}

@app.route('/')
def dashboard():
    """
    Dashboard showing user activity, statuses, time spent, and count of users in the office.
    """j
    now = datetime.now()

    # Count how many users are currently in the office
    users_in_office = len(current_users)

    # Prepare user status data for the second table
    user_status_data = []
    for tag, name in authorized_tags.items():
       
        
        user_status_data.append({
            "rfid": tag,
            "name": name,
            "status":"Blocked" if name in blocked_users else "Active",
            "in_office": "Yes" if name in current_users else "No"
        })

    # Calculate time spent for the first table
    user_logs_with_time = {}
    for name, logs in user_logs.items():
        time_spent = "N/A"
        if logs["first_entry"]:
            first_entry = datetime.strptime(logs["first_entry"], "%Y-%m-%d %H:%M:%S")
            if name in current_users:
                time_spent = str(now - first_entry).split('.')[0]
            elif logs["last_exit"]:
                last_exit = datetime.strptime(logs["last_exit"], "%Y-%m-%d %H:%M:%S")
                time_spent = str(last_exit - first_entry).split('.')[0]
        user_logs_with_time[name] = {
            **logs,
            "time_spent": time_spent
        }

    return render_template(
        'dashboard.html',
        current_users=current_users,
        user_logs=user_logs_with_time,
        user_status_data=user_status_data,
        users_in_office=users_in_office  # Pass the count of users in the office
    )

@app.route('/add-user', methods=['POST'])
def add_user():
    """
    Add a new user with RFID tag.
    """
    tag = request.form.get("tag")
    name = request.form.get("name")
    if tag and name:
        authorized_tags[tag] = name
        return redirect(url_for('dashboard'))
    return jsonify({"status": "error", "message": "Invalid data provided"}), 400

@app.route('/block-user', methods=['POST'])
def block_user():
    """
    Block a user by their name.
    """
    name = request.form.get("name")
    if name in authorized_tags.values():
        blocked_users.add(name)
    return redirect(url_for('dashboard'))

@app.route('/unblock-user', methods=['POST'])
def unblock_user():
    """
    Unblock a user by their name.
    """
    name = request.form.get("name")
    if name in blocked_users:
        blocked_users.remove(name)
    return redirect(url_for('dashboard'))

@app.route('/entry', methods=['POST'])
def entry():

    data = request.get_json()
    if not data or 'tag' not in data:
        return jsonify({"status": "error", "message": "Invalid request"}), 400

    tag = data['tag']
    now = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    if tag in authorized_tags and authorized_tags[tag] not in blocked_users:
        user = authorized_tags[tag]
        current_users[user] = now

        if user not in user_logs:
            user_logs[user] = {"first_entry": now, "last_exit": None}
        return jsonify({"status": "success", "message": "Entry logged", "user": user})
    else:
        return jsonify({"status": "error", "message": "Access Denied"})

@app.route('/exit', methods=['POST'])
def exit():

    data = request.get_json()
    if not data or 'tag' not in data:
        return jsonify({"status": "error", "message": "Invalid request"}), 400
    tag = data['tag']
    now = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    if tag in authorized_tags:
        user = authorized_tags[tag]
        if user in current_users:
            del current_users[user]
        user_logs[user]["last_exit"] = now
        return jsonify({"status": "success", "message": "Exit logged", "user": user})
    else:
        return jsonify({"status": "error", "message": "Access Denied"})
door_command = False

@app.route('/remote-open', methods=['POST'])
def remote_open():
    global door_command
    door_command = True  
    return jsonify({"status": "success", "message": "Door open command sent."})

@app.route('/check-door-command', methods=['GET'])
def check_door_command():
    global door_command
    response = {"open": door_command}
    door_command= False
    return jsonify(response)
if __name__ == '__main__':
    app.run(host='0.0.0.0', debug=True)
