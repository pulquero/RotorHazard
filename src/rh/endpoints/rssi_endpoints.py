from flask.blueprints import Blueprint


def createBlueprint(sample_listener):
    APP = Blueprint('rssi', __name__)

    @APP.route('/rssi')
    def rssi_data():
        rssis_by_node = sample_listener.get_rssis()
        lifetimes_by_node = sample_listener.get_lifetimes()
        return {'rssi': rssis_by_node, 'lifetime': lifetimes_by_node}

    return APP
