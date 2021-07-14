<?php if(!defined('OCTOMS')){header('HTTP/1.1 403');die('{"error":"forbidden"}');}
/**
 * @package   OctoMS
 * @copyright (c) 2011 Mark Jivko <stephino.team@gmail.com>
 * @license   GPL v3+ (https://gnu.org/licenses/gpl-3.0.txt)
 * 
 * Redistributions of files must retain the above copyright notice.
 * 
 * @version   0.0.1
 */
    
class session_cl {

    /**
     * The current session Id
     * @var string
     */
    var $id;

    /**
     * Class constructor
     * 
     * @return session_cl
     */
    public function __construct() {
        // Don't store the session Id in plain sight, encode it in our cookie instead
        @ini_set('session.use_cookies', 0);
    }

    /**
     * Check if the session was started
     * 
     * @example $this->session->is_started();
     * 
     * @return boolean - True if the session was started already
     */
    public function is_started() {
        return session_id() != '';
    }

    /**
     * Start the session; prevent cookie stealing and session sniffing
     * 
     * @example $this->session->start();
     * 
     * @return session_cl
     */
    public function start() {
        // First, get the cookie
        $cookie = $this->cookie_get();

        // Invalid cookie set?
        empty($cookie) ? $this->cookie_reset() : $cookie = (array) $cookie;

        // Set an empty user agent
        if (!isset($_SERVER['HTTP_USER_AGENT']))
            $_SERVER['HTTP_USER_AGENT'] = '';

        // Verify the user's cookie x-verify credentials
        if (isset($cookie['x-verify']) && $cookie['x-verify']->ua == md5($_SERVER['HTTP_USER_AGENT']) && $cookie['x-verify']->ip == ip2long($_SERVER['REMOTE_ADDR'])) {
            // The user passed; restore the session
            $this->id = $cookie['x-verify']->sessid;

            // Don't continue if the session was already started
            if (session_id() == $this->id)
                return $this;
        } else {
            // Destroy the session(s)
            session_start();
            session_unset();
            session_destroy();

            // Regenerate the session ID
            $this->id = md5(uniqid(mt_rand(), true));

            // Store it to the cookie
            $this->cookie_set(
                    array('x-verify' =>
                        array(
                            'sessid' => $this->id,
                            'ua' => md5($_SERVER['HTTP_USER_AGENT']),
                            'ip' => ip2long($_SERVER['REMOTE_ADDR'])
                        )
                    )
            );
        }

        // Launch the session with the generated Id
        session_id($this->id);
        session_start();
        return $this;
    }

    /**
     * Destroy the current session
     * 
     * @example $this->session->destroy();
     * 
     * @return session_cl
     * 
     */
    public function destroy() {
        // Clear all session data
        unset($_SESSION);

        // Remove the cookie
        $this->cookie_reset();
    }

    /**
     * Set a session value
     * 
     * @example 
     * // Set key=>value
     * $this->session->set('key','value');
     * // Set k1=>v1, k2=>v2
     * $this->session->set(array('k1'=>'v1','k2'=>'v2'));
     * 
     * @param $key and $value or $array
     * @return session_cl
     */
    public function set() {
        $params = func_get_args();
        if (isset($params[0]) && is_array($params[0])) {
            foreach ($params AS $k => $v) {
                $_SESSION[$k] = $v;
            }
        } elseif (isset($params[0]) && isset($params[1])) {
            $_SESSION[$params[0]] = $params[1];
        }

        return $this;
    }

    /**
     * Get a session value by key
     * 
     * @example $this->session->get('key');
     *     $this->session->get(array('key1','key2'));
     * 
     * @param string|array $keys
     * @return mixed
     */
    public function get($keys = null) {
        if (is_array($keys)) {
            $result = array();
            foreach ($keys AS $k) {
                $result[$k] = isset($_SESSION[$k]) ? $_SESSION[$k] : null;
            }
            return $result;
        } elseif (is_string($keys)) {
            return isset($_SESSION[$keys]) ? $_SESSION[$keys] : null;
        }

        return null;
    }

    /**
     * Unset a sessions value by key
     * 
     * @example $this->session->reset('key');
     *     $this->session->reset(array('key1','key2'));
     *     $this->session->reset(); # clean all session values
     * 
     * @param string|array $keys
     * @return session_cl
     */
    public function reset($keys = null) {
        if (is_array($keys)) {
            foreach ($keys AS $k) {
                unset($_SESSION[$k]);
            }
        } elseif (is_string($keys)) {
            unset($_SESSION[$keys]);
        } else {
            unset($_SESSION);
        }

        return $this;
    }

    /**
     * Set a flash variable or a set of flash variables
     * 
     * @example $this->session->flash_set('key','value');
     *     $this->session->flash_set(array('val1','val2'));
     * 
     * @param $key and $value or $array
     * @return session_cl 
     */
    public function flash_set() {
        $params = func_get_args();
        if (isset($params[0]) && is_array($params[0])) {
            foreach ($params[0] AS $k => $v) {
                $_SESSION['__flash__'][$k] = $v;
            }
        } elseif (isset($params[0]) && isset($params[1])) {
            $_SESSION['__flash__'][$params[0]] = $params[1];
        }

        return $this;
    }

    /**
     * Get a flash value (and, obviously, unset it)
     * 
     * @example $this->session->flash_get('key');
     *     $this->session->flash_get(array('key1','key2'));
     * 
     * @param string|array $keys
     * @return mixed
     */
    public function flash_get($keys = null) {
        if (is_array($keys)) {
            $result = array();
            foreach ($keys AS $k) {
                $result[$k] = isset($_SESSION['__flash__'][$k]) ? $_SESSION['__flash__'][$k] : null;
                unset($_SESSION['__flash__'][$k]);
            }
            return $result;
        } elseif (is_string($keys)) {
            if (isset($_SESSION['__flash__'][$keys])) {
                $temp = $_SESSION['__flash__'][$keys];
                unset($_SESSION['__flash__'][$keys]);
                return $temp;
            } else {
                return null;
            }
        }

        return null;
    }

    /**
     * Flash reset
     * 
     * @example $this->session->flash_reset('key');
     *     $this->session->flash_reset(array('key1','key2'));
     *     $this->session->flash_reset(); # unset all flash data
     * 
     * @param string|array $keys
     * @return session_cl
     * 
     */
    public function flash_reset($keys = null) {
        if (!empty($keys)) {
            $this->flash_get($keys);
        } else {
            unset($_SESSION['__flash__']);
        }

        return $this;
    }

    /**
     * Set the cookie (send it to the browser)
     * 
     * @example $this->session->cookie_set(array('a','b','c'));
     *     $this->session->cookie_set('value','cookieName',-100); #date from the past to kill the cookie 'cookieName'
     * 
     * @param mixed $value
     * @param string $name
     * @param int $expire
     * @return boolean
     */
    public function cookie_set($value, $name = COOKIE_NAME, $expire = COOKIE_EXPIRE) {

        // Set the cookie if the value is not empty
        return empty($value) ? false : setcookie($name, blowfish_encrypt(json_encode($value)), (time() + $expire), COOKIE_PATH, (false === ip2long($_SERVER['HTTP_HOST']) ? COOKIE_DOMAIN : null));
    }

    /**
     * Get the cookie values
     * 
     * @param string $name
     * @return string|boolean
     */
    public function cookie_get($name = COOKIE_NAME) {
        // Return available cookie data
        return isset($_COOKIE[$name]) ? json_decode(blowfish_decrypt($_COOKIE[$name])) : false;
    }

    /**
     * Reset the cookie (delete it)
     * 
     * @param string $name
     */
    public function cookie_reset($name = COOKIE_NAME) {
        // This cookie has expired
        if (isset($_COOKIE[$name])) {
            setcookie($name, '', time() - 3600);
            unset($_COOKIE[$name]);
        }
    }
}

/* End Of File <session.inc> */